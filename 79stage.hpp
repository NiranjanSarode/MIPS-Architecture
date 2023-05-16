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
	std::vector<std::string> command[9];	
	int temp[9]={-1,-1,-1,-1,-1,-1,-1,-1,-1};
	int temp1[9]={-1,-1,-1,-1,-1,-1,-1,-1,-1};
	int temp2[9]={-1,-1,-1,-1,-1,-1,-1,-1,-1};
	int temp3[9]={-1,-1,-1,-1,-1,-1,-1,-1,-1};
	std::string register_level_temp1[9]={"","","","","","","","",""};
	std::string register_level_temp2[9]={"","","","","","","","",""};
	std::string register_level_temp3[9]={"","","","","","","","",""};
	int x1 = 0;
	int x2 = 0;
	int x3 = 0;
	int x4 = 0;
	int x5 = 0;
	int x6 = 0;
	int x7 = 0;
	int x8 = 0;
	int x9 = 0;
	int add_stall = 0;
	int load_stall=0;
	int branch_selected = 0;
	int commandsize = 0;
	int jump_stall = 0;
	int jump_stall2 = 0;
	int regwrite_check1 = 0;
	int regwrite_check2 = 0;


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

	void fetch(int r){
		command[r] = commands[PCcurr[0]];
		++commandCount[PCcurr[0]];
		PCcurr[0] = PCnext[0];
		PCnext[0] = PCcurr[0] + 1;
		if(command[r][0] == "beq" || command[r][0] == "bne"){
			branch_selected = -5;
		}
		else if(command[r][0] == "j"){
			jump_stall = -3;
		}
	}

	void fetch2(int r){
		//do nothing
		// in actual deep pielining we many need to fetch in 2 parts 
	}

	void decode(int r){
		register_level_temp1[r] = command[r][1];
		register_level_temp2[r] = command[r][2];
		register_level_temp3[r] = command[r][3];
	}

	void decode2(int r){
		
		//do nothing
		// in actual deep pielining we many need to decode in 2 parts
	}

	void regread(int r){
		if(command[r][0] == "add" || command[r][0] == "sub" || command[r][0] == "mul" || command[r][0] == "slt"){
			temp1[r] = registerMap[register_level_temp1[r]];
			temp2[r] = registerMap[register_level_temp2[r]];
			temp3[r] = registerMap[register_level_temp3[r]];

			if(temp1[(r+6)%9] != -1){
				if(command[(r+6)%9][0] == "lw"){
					if(temp2[r] == temp1[(r+6)%9] || temp3[r] == temp1[(r+6)%9]){add_stall = 1;}
				}
				else if(command[(r+6)%9][0] == "sw"){
				}
				else if(command[(r+6)%9][0] == "beq" || command[(r+6)%9][0] == "bne"){
				}
			}
			if(temp1[(r+7)%9] != -1){
				if(command[(r+7)%9][0] == "lw"){
					if(temp2[r] == temp1[(r+7)%9] || temp3[r] == temp1[(r+7)%9]){add_stall = 2;}
				}
				else if(command[(r+7)%9][0] == "sw"){
				}
				else if(command[(r+7)%9][0] == "beq" || command[(r+7)%9][0] == "bne"){
				}
				else if(temp2[r] == temp1[(r+7)%9] || temp3[r] == temp1[(r+7)%9]){add_stall = 1;}
			}
			if(temp1[(r+8)%9] != -1){
				if(command[(r+8)%9][0] == "lw"){
					if(temp2[r] == temp1[(r+8)%9] || temp3[r] == temp1[(r+8)%9]){add_stall = 3;}
				}
				else if(command[(r+8)%9][0] == "sw"){
				}
				else if(command[(r+8)%9][0] == "beq" || command[(r+8)%9][0] == "bne"){
				}
				else if(temp2[r] == temp1[(r+8)%9] || temp3[r] == temp1[(r+8)%9]){add_stall = 2;}
			}
			

		}
		else if(command[r][0] == "lw" || command[r][0] == "sw"){
			temp1[r] = registerMap[register_level_temp1[r]];
			std::string location = register_level_temp2[r];
			int lparen = location.find('('), offset = stoi(lparen == 0 ? "0" : location.substr(0, lparen));
				std::string reg = location.substr(lparen + 1);
				reg.pop_back();
			temp2[r] = registerMap[reg];
			temp3[r] = offset;

			if(temp1[(r+7)%9] != -1){
				if(command[r][0] == "lw"){
					if(command[(r+7)%9][0] == "lw"){
						if(temp2[r] == temp1[(r+7)%9]){
							load_stall = 2;
							}
					}
					else if(command[(r+7)%9][0] == "sw"){
						if(temp2[r] == temp2[(r+7)%9]){
							load_stall = 2;
							}
					}
					else if(command[(r+7)%9][0] == "beq" || command[(r+7)%9][0] == "bne"){}
					else{
						if(temp2[r] == temp1[(r+7)%9]){
							load_stall = 1;
						}
					}			
				}
				else{
					if(command[(r+7)%9][0] == "lw"){
						if(temp1[r] == temp1[(r+7)%9] || temp2[r] == temp1[(r+7)%9]){
							load_stall = 2;
							}
					}
					else if(command[(r+7)%9][0] == "sw"){
						if(temp1[r] == temp2[(r+7)%9] || temp2[r] == temp2[(r+7)%9]){
							load_stall = 2;
							if(temp3[r] != temp3[(r+7)%9]){
								load_stall = 1;
							}
						}
					}
					else if(command[(r+7)%9][0] == "beq" || command[(r+7)%9][0] == "bne"){}
					else{
						if(temp1[r] == temp1[(r+7)%9] || temp2[r] == temp1[(r+7)%9]){
							load_stall = 1;
						}
					}
				}
			}
			
			if(temp1[(r+8)%9] != -1){
				if(command[r][0] == "lw"){
					if(command[(r+8)%9][0] == "lw"){
						if(temp2[r] == temp1[(r+8)%9]){
							load_stall = 3;
							}
					}
					else if(command[(r+8)%9][0] == "sw"){
						if(temp2[r] == temp2[(r+8)%9]){
							load_stall = 3;
							}
					}
					else if(command[(r+8)%9][0] == "beq" || command[(r+8)%9][0] == "bne"){}
					else{
						if(temp2[r] == temp1[(r+8)%9]){
							load_stall = 2;
						}
					}			
				}
				else{
					if(command[(r+8)%9][0] == "lw"){
						if(temp1[r] == temp1[(r+8)%9] || temp2[r] == temp1[(r+8)%9]){
							load_stall = 3;
							if(temp3[r] == temp3[(r+8)%9]){
								load_stall = 0;
							}
						}
					}
					else if(command[(r+8)%9][0] == "sw"){
						if(temp1[r] == temp2[(r+8)%9] || temp2[r] == temp2[(r+8)%9]){
							load_stall = 3;
							if(temp3[r] != temp3[(r+8)%9]){
								load_stall = 0;
							}
						}
					}
					else if(command[(r+8)%9][0] == "beq" || command[(r+8)%9][0] == "bne"){}
					else{
						if(temp1[r] == temp1[(r+8)%9] || temp2[r] == temp1[(r+8)%9]){
							load_stall = 2;
						}
					}
				}
			}
		}
		else if(command[r][0] == "beq" || command[r][0] == "bne"){
			temp1[r] = registerMap[register_level_temp1[r]];
			temp2[r] = registerMap[register_level_temp2[r]];
			temp3[r] = address[register_level_temp3[r]];

			if(temp1[(r+6)%9] != -1){
				if(command[(r+6)%9][0] == "lw"){
					if(temp1[r] == temp1[(r+6)%9] || temp2[r] == temp1[(r+6)%9]){add_stall = 1;}
				}
				else if(command[(r+3)%5][0] == "sw"){
					if(temp1[r] == temp2[(r+6)%9] || temp2[r] == temp2[(r+6)%9]){add_stall = 1;}
				}
				else if(command[(r+4)%5][0] == "beq" || command[(r+4)%5][0] == "bne"){}
			}
			if(temp1[(r+7)%9] != -1){
				if(command[(r+7)%9][0] == "lw"){
					if(temp1[r] == temp1[(r+7)%9] || temp2[r] == temp1[(r+7)%9]){add_stall = 2;}
				}
				else if(command[(r+7)%9][0] == "sw"){
					if(temp1[r] == temp2[(r+7)%9] || temp2[r] == temp2[(r+7)%9]){add_stall = 2;}
				}
				else if(command[(r+7)%9][0] == "beq" || command[(r+7)%9][0] == "bne"){}
			}
			if(temp1[(r+8)%9] != -1){
				if(command[(r+8)%9][0] == "lw"){
					if(temp1[r] == temp1[(r+8)%9] || temp2[r] == temp1[(r+8)%9]){add_stall = 3;}
				}
				else if(command[(r+8)%9][0] == "sw"){
					if(temp1[r] == temp2[(r+8)%9] || temp2[r] == temp2[(r+8)%9]){add_stall = 3;}
				}
				else if(command[(r+8)%9][0] == "beq" || command[(r+8)%9][0] == "bne"){}
			}
		}
		else if(command[r][0]=="j"){
			temp1[r] = address[register_level_temp1[r]];
			commandsize = commandsize + PCcurr[0] - temp1[r];
			PCnext[0] = temp1[r] + 1;
			PCcurr[0] = temp1[r];
		}
		else if(command[r][0] == "addi"){
			temp1[r] = registerMap[register_level_temp1[r]];
			temp2[r] = registerMap[register_level_temp2[r]];
			temp3[r] = std::stoi(register_level_temp3[r]);

			if(temp1[(r+6)%9] != -1){
				if(command[(r+6)%9][0] == "lw"){
					if(temp2[r] == temp1[(r+6)%9]){add_stall = 1;}
				}
				else if(command[(r+6)%9][0] == "sw"){
				}
				else if(command[(r+6)%9][0] == "beq" || command[(r+6)%9][0] == "bne"){
				}
			}
			if(temp1[(r+7)%9] != -1){
				if(command[(r+7)%9][0] == "lw"){
					if(temp2[r] == temp1[(r+7)%9]){add_stall = 2;}
				}
				else if(command[(r+7)%9][0] == "sw"){
				}
				else if(command[(r+7)%9][0] == "beq" || command[(r+7)%9][0] == "bne"){
				}
				else if(temp2[r] == temp1[(r+7)%9]){add_stall = 1;}
			}
			if(temp1[(r+8)%9] != -1){
				if(command[(r+8)%9][0] == "lw"){
					if(temp2[r] == temp1[(r+8)%9]){add_stall = 3;}
				}
				else if(command[(r+8)%9][0] == "sw"){
				}
				else if(command[(r+8)%9][0] == "beq" || command[(r+8)%9][0] == "bne"){
				}
				else if(temp2[r] == temp1[(r+8)%9]){add_stall = 2;}
			}

		}
	}

	void ALU(int r){
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
			int address = registers[temp2[r]] + temp3[r];
				temp[r] = address / 4;
		}
		else if(command[r][0]=="slt"){
			if(registers[temp2[r]] < registers[temp3[r]]){temp[r] = 1;}
			else{temp[r] = 0;}
		}
		else if(command[r][0]=="beq"){
			if(registers[temp1[r]] == registers[temp2[r]]){
				int p = temp3[r] - PCcurr[0]+1;
				commandsize = commandsize - p + 1;
				PCnext[0] = temp3[r];
				PCcurr[0] = temp3[r];
				}
		}
		else if(command[r][0]=="bne"){
			if(registers[temp1[r]] != registers[temp2[r]]){
				int p = temp3[r] - PCcurr[0]+1;
				commandsize = commandsize - p + 1;
				PCnext[0] = temp3[r];
				PCcurr[0] = temp3[r];
				}
		}
	}



	void data_memory(int r){
		if(command[r][0] == "sw"){
		data[temp[r]] = registers[temp1[r]];
		}
		else if(command[r][0] == "lw"){
			
			temp[r] = data[temp[r]];
		}
	}

	void data_memory2(int r){
		//do nothing
	}


	void regwrite(int r){
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
		int print = 1;
		printRegisters(0);
		// std::cout<<"0"<<std::endl;
		int clockCycles = 0;
		commandsize = commands.size();
		while (x9 < commandsize)
		{
			++clockCycles;

			//*STAGE 9
			if(x8 > x9 && x9 < commandsize && (command[x9%9][0]== "lw" || command[x9%9][0]== "sw")){
				
				regwrite((x9) %9);
				if(command[(x9)%9][0] == "sw"){
						print = 0;
					}
					else{
						print = 1;
					}
				x9++;
			}
			else if(x8>x9 && x9<commandsize){
				x9++;
			}

			//*STAGE 7&8
			if(temp1[x8%9] == -1){}
			else{
			if(command[x8%9][0]=="lw" || command[x8%9][0]=="sw"){			
				if(x7 > x8 && x8 < commandsize){
					data_memory2((x8)%9);
					x8++;
				}
			}
			else if(x7>x8 && x8 < commandsize){
				x8++;
			}
			}

			if(temp1[x7%9] == -1){}
			else{
			if(command[x7%9][0]=="lw" || command[x7%9][0]=="sw"){
				if(load_stall==0){
				if(x6 > x7 && x7 < commandsize){
					data_memory((x7)%9);
					x7++;
				}
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
						if(data[temp[(x9-1)%9]] == 0){
							std::cout<<"0"<<std::endl;
						}
						else{
							std::cout<<"1 "<<temp[(x9-1)%9]<<" "<<data[temp[(x9-1)%9]]<<std::endl;
						}
						print = 1;
					}
					printRegisters(clockCycles);
					
					continue;
					}
				}
				if(x6 > x7 && x7 < commandsize && command[x7%9][0]!= "lw" && command[x7%9][0]!= "sw"){
					regwrite(x7%9);
					x7++;
					if(x7 == commandsize){
						x9 = x7;
					}
				}
			}
			if(x5 > x6 && x6 < commandsize){
				if(add_stall == 0){
					ALU((x6) %9);
					x6++;
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
						if(data[temp[(x9-1)%9]] == 0){
							std::cout<<"0"<<std::endl;
						}
						else{
							std::cout<<"1 "<<temp[(x9-1)%9]<<" "<<data[temp[(x9-1)%9]]<<std::endl;
						}
						print = 1;
					}
					printRegisters(clockCycles);
					continue;
				}
			}
			//*STAGE 5
			if(x4 > x5 && x5 < commandsize && (branch_selected >= 0 ||  branch_selected < -1)){
				regread((x5) %9);
				x5++;
			}
			//*STAGE 3&4
			if(x3 > x4 && x4 < commandsize && (branch_selected >= 0 ||  branch_selected < -2)){
				decode2((x4) %9);
				x4++;
			}
			if(x2 > x3 && x3 < commandsize	&& (branch_selected >= 0 ||  branch_selected < -3) && (jump_stall >=0 || jump_stall < -1)){
				decode((x3) %9);
				x3++;
			}
			//*STAGE 1&2
			if(x1 > x2 && x2 < commandsize && (branch_selected >= 0 || branch_selected < -4) && (jump_stall >=0 || jump_stall < -2)){
				fetch2((x2) %9);
				x2++;
			}
			if(x1 < commandsize && branch_selected >= 0 && jump_stall >=0){
				fetch((x1) %9);
				x1++;
			}
			else if(branch_selected < 0){
				branch_selected++;
				if(print == 1){
						std::cout<<"0"<<std::endl;
					}
					else{
						if(data[temp[(x9-1)%9]] == 0){
							std::cout<<"0"<<std::endl;
						}
						else{
							std::cout<<"1 "<<temp[(x9-1)%9]<<" "<<data[temp[(x9-1)%9]]<<std::endl;
						}
						print = 1;
					}
				printRegisters(clockCycles);
				continue;
			}
			else if(jump_stall < 0){
				jump_stall++;
				if(print == 1){
						std::cout<<"0"<<std::endl;
					}
					else{
						if(data[temp[(x9-1)%9]] == 0){
							std::cout<<"0"<<std::endl;
						}
						else{
							std::cout<<"1 "<<temp[(x9-1)%9]<<" "<<data[temp[(x9-1)%9]]<<std::endl;
						}
						print = 1;
					}
				printRegisters(clockCycles);
				continue;
			}
			if(print == 1){
						std::cout<<"0"<<std::endl;
					}
					else{
						if(data[temp[(x9-1)%9]] == 0){
							std::cout<<"0"<<std::endl;
						}
						else{
							std::cout<<"1 "<<temp[(x9-1)%9]<<" "<<data[temp[(x9-1)%9]]<<std::endl;
						}
						print = 1;
					}
			printRegisters(clockCycles);
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
