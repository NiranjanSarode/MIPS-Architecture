compile: 5stage.cpp 5stage.hpp 5stage_bypass.cpp 5stage_bypass.hpp 79stage.cpp 79stage.hpp 79stage_bypass.hpp 79stage_bypass.hpp
	g++ 5stage.cpp 5stage.hpp -o 5STAGE
	g++ 5stage_bypass.cpp 5stage_bypass.hpp -o 5STAGE_BYPASS
	g++ 79stage.cpp 79stage.hpp -o 79STAGE
	g++ 79stage_bypass.cpp 79stage_bypass.hpp -o 79STAGE_BYPASS
	g++ BranchPredictor.cpp BranchPredictor.hpp -o BRANCH_PREDICTOR
	

run_5stage: 
	./5STAGE input.asm

run_5stage_bypass: 
	./5STAGE_BYPASS input.asm

run_79stage: 
	./79STAGE input.asm

run_79stage_bypass: 
	./79STAGE_BYPASS input.asm

run_branch_predictor: input.txt
	./BRANCH_PREDICTOR 

clean:
	rm -rf 5STAGE 5STAGE_BYPASS 79STAGE 79STAGE_BYPASS BRANCH_PREDICTOR