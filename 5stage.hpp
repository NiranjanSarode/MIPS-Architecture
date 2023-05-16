#ifndef __MIPS_PROCESSOR_HPP__
#define __MIPS_PROCESSOR_HPP__

#include <unordered_map>
#include <string>
#include <functional>
#include <vector>
#include <ctime>
#include <fstream>
#include <exception>
#include <iostream>
#include <boost/tokenizer.hpp>

struct MIPS_Architecture
{
	int registers[32] = {0}, PCcurr[5]={0,0,0,0,0}, PCnext[5]={1,0,0,0,0};
	std::unordered_map<std::string, std::function<int(MIPS_Architecture &, std::string, std::string, std::string)>> instructions;
	std::unordered_map<std::string, int> registerMap, address;
	static const int MAX = (1 << 20);
	int data[MAX >> 2] = {0};
	std::vector<std::vector<std::string>> commands;
	std::vector<int> commandCount;
	std::vector<std::string> command[5];	
	int temp[5]={-1,-1,-1,-1,-1};
	int temp1[5]={-1,-1,-1,-1,-1};
	int temp2[5]={-1,-1,-1,-1,-1};
	int temp3[5]={-1,-1,-1,-1,-1};
	int x1 = 0;
	int x2 = 0;
	int x3 = 0;
	int x4 = 0;
	int x5 = 0;
	int add_stall = 0;
	int load_stall=0;
	int branch_selected = 0;
	int commandsize = 0;
	int jump_stall = 0;

	enum exit_code
	{
		SUCCESS = 0,
		INVALID_REGISTER,
		INVALID_LABEL,
		INVALID_ADDRESS,
		SYNTAX_ERROR,
		MEMORY_ERROR
	};

	// constructor to initialise the instruction set
	MIPS_Architecture(std::ifstream &file)
	{
		for (int i = 0; i < 32; ++i)
			registerMap["$" + std::to_string(i)] = i;
		registerMap["$zero"] = 0;
		registerMap["$at"] = 1;
		registerMap["$v0"] = 2;
		registerMap["$v1"] = 3;
		for (int i = 0; i < 4; ++i)
			registerMap["$a" + std::to_string(i)] = i + 4;
		for (int i = 0; i < 8; ++i)
			registerMap["$t" + std::to_string(i)] = i + 8, registerMap["$s" + std::to_string(i)] = i + 16;
		registerMap["$t8"] = 24;
		registerMap["$t9"] = 25;
		registerMap["$k0"] = 26;
		registerMap["$k1"] = 27;
		registerMap["$gp"] = 28;
		registerMap["$sp"] = 29;
		registerMap["$s8"] = 30;
		registerMap["$ra"] = 31;

		constructCommands(file);
		commandCount.assign(commands.size(), 0);
	}

	void ifexecute(int r){
		command[r] = commands[PCcurr[0]];
		++commandCount[PCcurr[0]];
		PCcurr[0] = PCnext[0];
		PCnext[0] = PCcurr[0] + 1;
	}

	void idexecute(int r){
		if(command[r][0] == "add" || command[r][0] == "sub" || command[r][0] == "mul" || command[r][0] == "slt"){
			temp1[r] = registerMap[command[r][1]];
			temp2[r] = registerMap[command[r][2]];
			temp3[r] = registerMap[command[r][3]];

			if(temp1[(r+3)%5] != -1){
				if(command[(r+3)%5][0] == "lw"){
					if(temp2[r] == temp1[(r+3)%5] || temp3[r] == temp1[(r+3)%5]){add_stall = 1;}
				}
				else if(command[(r+3)%5][0] == "sw"){
				}
				else if(command[(r+3)%5][0] == "beq" || command[(r+3)%5][0] == "bne"){ add_stall = 0;
				}
				else if(temp2[r] == temp1[(r+3)%5] || temp3[r] == temp1[(r+3)%5]){add_stall = 1;}
			}
			if(temp1[(r+4)%5] != -1){
				if(command[(r+4)%5][0] == "lw"){
					if(temp2[r] == temp1[(r+4)%5] || temp3[r] == temp1[(r+4)%5]){add_stall = 2;}
				}
				else if(command[(r+4)%5][0] == "sw"){
				}
				else if(command[(r+4)%5][0] == "beq" || command[(r+4)%5][0] == "bne"){{ add_stall = 0;
				}
				}
				else if(temp2[r] == temp1[(r+4)%5] || temp3[r] == temp1[(r+4)%5]){add_stall = 2;}
				
			}
		}
		else if(command[r][0] == "addi"){
			temp1[r] = registerMap[command[r][1]];
			temp2[r] = registerMap[command[r][2]];
			temp3[r] = std::stoi(command[r][3]);
			if(temp1[(r+3)%5] != -1){
				if(command[(r+3)%5][0] == "lw"){
					if(temp2[r] == temp1[(r+3)%5]){add_stall = 1;}
				}
				else if(command[(r+3)%5][0] == "sw"){
				}
				else if(command[(r+3)%5][0] == "beq" || command[(r+3)%5][0] == "bne"){	add_stall = 0;
				}
				else if(temp2[r] == temp1[(r+3)%5]){add_stall = 1;}
			}
			if(temp1[(r+4)%5] != -1){
				if(command[(r+4)%5][0] == "lw"){
					if(temp2[r] == temp1[(r+4)%5]){add_stall = 2;}
				}
				else if(command[(r+4)%5][0] == "sw"){
				}
				else if(command[(r+4)%5][0] == "beq" || command[(r+4)%5][0] == "bne"){ add_stall = 0;
				}
				else{
					if(temp2[r] == temp1[(r+4)%5]){add_stall = 2;}
				}
			}
		}
		else if(command[r][0]=="lw" || command[r][0]=="sw"){
			temp1[r] = registerMap[command[r][1]];
			std::string location = command[r][2];
			int lparen = location.find('('), offset = stoi(lparen == 0 ? "0" : location.substr(0, lparen));
				std::string reg = location.substr(lparen + 1);
				reg.pop_back();
			temp2[r] = registerMap[reg];
			temp3[r] = offset;
			if(temp1[(r+3)%5] != -1){
				if(command[r][0] == "lw"){
					if(command[(r+3)%5][0] == "lw"){
						if(temp2[r] == temp1[(r+3)%5]){
							load_stall = 1;
							}
					}
					else if(command[(r+3)%5][0] == "sw"){
						if(temp2[r] == temp2[(r+3)%5]){
							load_stall = 1;
							}
					}
					else if(command[(r+3)%5][0] == "beq" || command[(r+3)%5][0] == "bne"){ load_stall = 0;}
					else{
						if(temp2[r] == temp1[(r+3)%5]){
							load_stall = 1;
						}
					}			
				}
				else{
					if(command[(r+3)%5][0] == "lw"){
						if(temp1[r] == temp1[(r+3)%5] || temp2[r] == temp1[(r+3)%5]){
							load_stall = 1;
							}
					}
					else if(command[(r+3)%5][0] == "sw"){
						if(temp1[r] == temp2[(r+3)%5] || temp2[r] == temp2[(r+3)%5]){
							load_stall = 1;
							if(temp3[r] != temp3[(r+3)%5]){
								load_stall = 0;
							}
						}
					}
					else if(command[(r+3)%5][0] == "beq" || command[(r+3)%5][0] == "bne"){ load_stall = 0;}
					else{
						if(temp1[r] == temp1[(r+3)%5] || temp2[r] == temp1[(r+3)%5]){
							load_stall = 1;
						}
					}
				}
			}
			if(temp1[(r+4)%5] != -1){
				if(command[r][0] == "lw"){
					if(command[(r+4)%5][0] == "lw"){
						if(temp2[r] == temp1[(r+4)%5]){
							load_stall = 2;
							}
					}
					else if(command[(r+4)%5][0] == "sw"){
						if(temp2[r] == temp2[(r+4)%5]){
							load_stall = 2;
							}
					}
					else if(command[(r+4)%5][0] == "beq" || command[(r+4)%5][0] == "bne"){load_stall = 0;}
					else{
						if(temp2[r] == temp1[(r+4)%5]){
							load_stall = 2;
						}
					}	
				}
				else{
					if(command[(r+4)%5][0] == "lw"){
						if(temp1[r] == temp1[(r+4)%5] || temp2[r] == temp1[(r+4)%5]){
							load_stall = 2;
							}
					}
					else if(command[(r+4)%5][0] == "sw"){
						if(temp1[r] == temp2[(r+4)%5] || temp2[r] == temp2[(r+4)%5]){
							load_stall = 2;
							if(temp3[r] != temp3[(r+4)%5]){
								load_stall = 1;
							}
						}
					}
					else if(command[(r+4)%5][0] == "beq" || command[(r+4)%5][0] == "bne"){load_stall = 0;}
					else{
						if(temp1[r] == temp1[(r+4)%5] || temp2[r] == temp1[(r+4)%5]){
							load_stall = 2;
						}
					}
				}
			}
		}
		else if(command[r][0]=="beq"){
			temp1[r] = registerMap[command[r][1]];
			temp2[r] = registerMap[command[r][2]];
			temp3[r] = address[command[r][3]];
			jump_stall = 1;
			if(temp1[(r+3)%5] != -1){
				if(command[(r+3)%5][0] == "lw"){
					if(temp1[r] == temp1[(r+3)%5] || temp2[r] == temp1[(r+3)%5]){add_stall = 1;}
				}
				else if(command[(r+3)%5][0] == "sw"){
					if(temp1[r] == temp2[(r+3)%5] || temp2[r] == temp2[(r+3)%5]){add_stall = 1;}
				}
				else if(command[(r+4)%5][0] == "beq" || command[(r+4)%5][0] == "bne"){ add_stall = 0;}
				else if(temp1[r] == temp1[(r+3)%5] || temp2[r] == temp1[(r+3)%5]){add_stall = 1;}
				
			}
			if(temp1[(r+4)%5] != -1){
				if(command[(r+4)%5][0] == "lw"){
					if(temp1[r] == temp1[(r+4)%5] || temp2[r] == temp1[(r+4)%5]){add_stall = 2;}
				}
				else if(command[(r+4)%5][0] == "sw"){
					if(temp1[r] == temp2[(r+4)%5] || temp2[r] == temp2[(r+4)%5]){add_stall = 2;}
				}
				else if(command[(r+4)%5][0] == "beq" || command[(r+4)%5][0] == "bne"){add_stall = 0;}
				else{
					if(temp1[r] == temp1[(r+4)%5] || temp2[r] == temp1[(r+4)%5]){add_stall = 2;}
				}
			
			}
		}
		else if(command[r][0]=="bne"){
			temp1[r] = registerMap[command[r][1]];
			temp2[r] = registerMap[command[r][2]];
			temp3[r] = address[command[r][3]];
			jump_stall = 1;
		if(temp1[(r+3)%5] != -1){
				if(command[(r+3)%5][0] == "lw"){
					if(temp1[r] == temp1[(r+3)%5] || temp2[r] == temp1[(r+3)%5]){add_stall = 1;}
				}
				else if(command[(r+3)%5][0] == "sw"){
					if(temp1[r] == temp2[(r+3)%5] || temp2[r] == temp2[(r+3)%5]){add_stall = 1;}
				}
				else if(command[(r+3)%5][0] == "beq" || command[(r+3)%5][0] == "bne"){ add_stall = 0;}
				else if(temp1[r] == temp1[(r+3)%5] || temp2[r] == temp1[(r+3)%5]){add_stall = 1;}
				
			}
			if(temp1[(r+4)%5] != -1){
				if(command[(r+4)%5][0] == "lw"){
					if(temp1[r] == temp1[(r+4)%5] || temp2[r] == temp1[(r+4)%5]){add_stall = 2;}
				}
				else if(command[(r+4)%5][0] == "sw"){
					if(temp1[r] == temp2[(r+4)%5] || temp2[r] == temp2[(r+4)%5]){add_stall = 2;}
				}
				else if(command[(r+4)%5][0] == "beq" || command[(r+4)%5][0] == "bne"){add_stall = 0;}
				else{
					if(temp1[r] == temp1[(r+4)%5] || temp2[r] == temp1[(r+4)%5]){add_stall = 2;}
				}
			
			}
		}
		else if(command[r][0]=="j"){
			temp1[r] = address[command[r][1]];
			jump_stall = 1;
			commandsize = commandsize + PCcurr[0] - temp1[r];
			PCnext[0] = temp1[r] + 1;
			PCcurr[0] = temp1[r];
		}
	
	}
	void exexecute(int r){
		if(command[r][0] == "add"){
			temp[r] = registers[temp2[r]] + registers[temp3[r]];
		}
		else if(command[r][0] == "addi"){
			temp[r] = registers[temp2[r]] + temp3[r];
		}
		else if(command[r][0] == "sub"){
			temp[r] = registers[temp2[r]] - registers[temp3[r]];
		}
		else if(command[r][0] == "mul"){
			temp[r] = registers[temp2[r]] * registers[temp3[r]];
		}
		else if(command[r][0] == "lw" || command[r][0] == "sw"){
		}
		else if(command[r][0]=="slt"){
			if(registers[temp2[r]] < registers[temp3[r]]){temp[r] = 1;}
			else{temp[r] = 0;}
		}
		else if(command[r][0]=="beq"){
			branch_selected = 1;
			if(registers[temp1[r]] == registers[temp2[r]]){
				int p = temp3[r] - PCcurr[0]+1;
				commandsize = commandsize - p + 1;
				PCnext[0] = temp3[r];
				PCcurr[0] = temp3[r];
				}
		}
		else if(command[r][0]=="bne"){
			branch_selected = 1;
			if(registers[temp1[r]] != registers[temp2[r]]){
				int p = temp3[r] - PCcurr[0]+1;
				commandsize = commandsize - p + 1;
				PCnext[0] = temp3[r];
				PCcurr[0] = temp3[r];
				}
		}
	}

	void memexecute(int r){
		if(command[r][0] == "sw"){
			int address = registers[temp2[r]] + temp3[r];
			// std::cout << "address: " << address << " " << registers[temp2[r]] << " " << temp3[r] << std::endl;
			temp[r] = address / 4;
			data[temp[r]] = registers[temp1[r]];
		}
		else if(command[r][0] == "lw"){
			int address = registers[temp2[r]] + temp3[r];
			// std::cout << "address: " << address << " " << registers[temp2[r]] << " " << temp3[r] << std::endl;
			temp[r] = address / 4;
			temp[r] = data[temp[r]];
		}
		if(command[r][0] == "j"){
			if(add_stall>0){add_stall--;}
			if(load_stall>0){load_stall--;}
		}
	}

	void wbexecute(int r){
		if(command[r][0] == "add" || command[r][0] == "sub" || command[r][0] == "mul" || command[r][0] == "addi" || command[r][0] == "slt"){
			registers[temp1[r]] = temp[r];
		}
		else if(command[r][0] == "lw"){
			registers[temp1[r]] = temp[r];
		}
		
	}
	void executeunpipeline()
	{
		

		if (commands.size() >= MAX / 4)
		{
			handleExit(MEMORY_ERROR, 0, 0);
			return;
		}
		printRegisters(0);
		// std::cout<<"0"<<std::endl;
		int clockCycles = 0;
		int print = 1;
		commandsize = commands.size();
		while (x5 < commandsize)
		{
			++clockCycles;
			if(x4 > x5 && x5 < commandsize){
				if(command[(x5)%5][0] == "j"){
					if(add_stall>0){add_stall--;}
					if(load_stall>0){load_stall--;}
				}
				else if(command[(x5)%5][0] == "sw"){
					print = 0;
				}
				else{
					print = 1;
				}
				wbexecute((x5) %5);
				x5++;
			}
			
			
			if(x3 > x4 && x4 < commandsize){
				
				if(load_stall==0 || (command[x4%5][0] != "lw" && command[x4%5][0] != "sw")){
					memexecute((x4) %5);
					x4++;
				}
				else{
					load_stall--;
					if(add_stall>0){
						add_stall--;
					}
					if(print == 1){
						std::cout<<"0"<<std::endl;
					}
					else{
						if(data[temp[(x5-1)%5]] == 0){
							std::cout<<"0"<<std::endl;
						}
						else{
							std::cout<<"1 "<<temp[(x5-1)%5]<<" "<<data[temp[(x5-1)%5]]<<std::endl;
						}
						print = 1;
					}
					printRegisters(clockCycles);
					
					continue;
				}
			}

			if(x2 > x3 && x3 < commandsize){
				if(add_stall == 0){
					exexecute((x3) %5);
					x3++;
				}
				else{
					add_stall--;
					if(load_stall>0){
						load_stall--;
					}
					if(print == 1){
						std::cout<<"0"<<std::endl;
					}
					else{
						if(data[temp[(x5-1)%5]] == 0){
							std::cout<<"0"<<std::endl;
						}
						else{
							std::cout<<"1 "<<temp[(x5-1)%5]<<" "<<data[temp[(x5-1)%5]]<<std::endl;
						}
						print = 1;
					}
					printRegisters(clockCycles);
					continue;
				}
			}
			if(branch_selected == 1) {
				branch_selected = 0;
				if(print == 1){
						std::cout<<"0"<<std::endl;
					}
					else{
						if(data[temp[(x5-1)%5]] == 0){
							std::cout<<"0"<<std::endl;
						}
						else{
							std::cout<<"1 "<<temp[(x5-1)%5]<<" "<<data[temp[(x5-1)%5]]<<std::endl;
						}
						print = 1;
					}
				printRegisters(clockCycles);
			}
			else{
			if(x1 > x2 && x2 < commandsize){
				idexecute((x2) %5);
				x2++;
			}
			if(jump_stall == 1){
				jump_stall = 0;
			}
			else if(x1 < commandsize){
				ifexecute((x1) %5);
				x1++;
			}
			if(print == 1){
						std::cout<<"0"<<std::endl;
					}
					else{
						if(data[temp[(x5-1)%5]] == 0){
							std::cout<<"0"<<std::endl;
						}
						else{
							std::cout<<"1 "<<temp[(x5-1)%5]<<" "<<data[temp[(x5-1)%5]]<<std::endl;
						}
						print = 1;
					}
			printRegisters(clockCycles);
			}
		}
		std::cout<<"0"<<std::endl;
	}
	int locateAddress(std::string location)
	{
		if (location.back() == ')')
		{
			try
			{
				int lparen = location.find('('), offset = stoi(lparen == 0 ? "0" : location.substr(0, lparen));
				std::string reg = location.substr(lparen + 1);
				reg.pop_back();
				if (!checkRegister(reg))
					return -3;
				int address = registers[registerMap[reg]] + offset;
				if (address % 4 || address < int(4 * commands.size()) || address >= MAX)
					return -3;
				return address / 4;
			}
			catch (std::exception &e)
			{
				return -4;
			}
		}
		try
		{
			int address = stoi(location);
			if (address % 4 || address < int(4 * commands.size()) || address >= MAX)
				return -3;
			return address / 4;
		}
		catch (std::exception &e)
		{
			return -4;
		}
	}
	// checks if label is valid
	inline bool checkLabel(std::string str)
	{
		return str.size() > 0 && isalpha(str[0]) && all_of(++str.begin(), str.end(), [](char c)
														   { return (bool)isalnum(c); }) &&
			   instructions.find(str) == instructions.end();
	}

	// checks if the register is a valid one
	inline bool checkRegister(std::string r)
	{
		return registerMap.find(r) != registerMap.end();
	}

	// checks if all of the registers are valid or not
	bool checkRegisters(std::vector<std::string> regs)
	{
		return std::all_of(regs.begin(), regs.end(), [&](std::string r)
						   { return checkRegister(r); });
	}

	/*
		handle all exit codes:
		0: correct execution
		1: register provided is incorrect
		2: invalid label
		3: unaligned or invalid address
		4: syntax error
		5: commands exceed memory limit
	*/
	void handleExit(exit_code code, int cycleCount,int r)
	{
		std::cout << '\n';
		switch (code)
		{
		case 1:
			std::cerr << "Invalid register provided or syntax error in providing register\n";
			break;
		case 2:
			std::cerr << "Label used not defined or defined too many times\n";
			break;
		case 3:
			std::cerr << "Unaligned or invalid memory address specified\n";
			break;
		case 4:
			std::cerr << "Syntax error encountered\n";
			break;
		case 5:
			std::cerr << "Memory limit exceeded\n";
			break;
		default:
			break;
		}
		if (code != 0)
		{
			std::cerr << "Error encountered at:\n";
			for (auto &s : commands[PCcurr[r]])
				std::cerr << s << ' ';
			std::cerr << '\n';
		}
		std::cout << "\nFollowing are the non-zero data values:\n";
		for (int i = 0; i < MAX / 4; ++i)
			if (data[i] != 0)
				std::cout << 4 * i << '-' << 4 * i + 3 << std::hex << ": " << data[i] << '\n'
						  << std::dec;
		std::cout << "\nTotal number of cycles: " << cycleCount << '\n';
		std::cout << "Count of instructions executed:\n";
		for (int i = 0; i < (int)commands.size(); ++i)
		{
			std::cout << commandCount[i] << " times:\t";
			for (auto &s : commands[i])
				std::cout << s << ' ';
			std::cout << '\n';
		}
	}

	// parse the command assuming correctly formatted MIPS instruction (or label)
	void parseCommand(std::string line)
	{
		// strip until before the comment begins
		line = line.substr(0, line.find('#'));
		std::vector<std::string> command;
		boost::tokenizer<boost::char_separator<char>> tokens(line, boost::char_separator<char>(", \t"));
		for (auto &s : tokens)
			command.push_back(s);
		// empty line or a comment only line
		if (command.empty())
			return;
		else if (command.size() == 1)
		{
			std::string label = command[0].back() == ':' ? command[0].substr(0, command[0].size() - 1) : "?";
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command.clear();
		}
		else if (command[0].back() == ':')
		{
			std::string label = command[0].substr(0, command[0].size() - 1);
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command = std::vector<std::string>(command.begin() + 1, command.end());
		}
		else if (command[0].find(':') != std::string::npos)
		{
			int idx = command[0].find(':');
			std::string label = command[0].substr(0, idx);
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command[0] = command[0].substr(idx + 1);
		}
		else if (command[1][0] == ':')
		{
			if (address.find(command[0]) == address.end())
				address[command[0]] = commands.size();
			else
				address[command[0]] = -1;
			command[1] = command[1].substr(1);
			if (command[1] == "")
				command.erase(command.begin(), command.begin() + 2);
			else
				command.erase(command.begin(), command.begin() + 1);
		}
		if (command.empty())
			return;
		if (command.size() > 4)
			for (int i = 4; i < (int)command.size(); ++i)
				command[3] += " " + command[i];
		command.resize(4);
		commands.push_back(command);
	}

	// construct the commands vector from the input file
	void constructCommands(std::ifstream &file)
	{
		std::string line;
		while (getline(file, line))
			parseCommand(line);
		file.close();
	}
	// print the register data in hexadecimal
	void printRegisters(int clockCycle)
	{
		std::cout << std::dec;
		for (int i = 0; i < 32; ++i)
			std::cout << registers[i] << ' ';
		std::cout << std::dec << '\n';
	}

};

#endif
