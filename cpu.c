/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
#define ENABLE_DEBUG_MESSAGES 1

/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
int flag = 0,busy = 0,BZ_flag = 0,BZ_pc = 0, BZ_stall=0, BZ_reg=0, BZ_pointer=0,bz_temp=0,bz_temp2=0,Temp,counter=0,Halt_flag=0;
int last_pc=0;
	
APEX_CPU*
APEX_cpu_init(const char* filename)
{
  if (!filename) {
    return NULL;
  }

  APEX_CPU* cpu = malloc(sizeof(*cpu));
  if (!cpu) {
    return NULL;
  }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 32);
  memset(cpu->regs_valid, 1, sizeof(int) * 32);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);
  cpu -> clock = 0;
  for (int i =0; i<=31;i++)
  {
	  cpu->regs_valid[i]=1;
  }
  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory) {
    free(cpu);
    return NULL;
  }

  if (ENABLE_DEBUG_MESSAGES) {
    fprintf(stderr,
            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i) {
      printf("%-9s %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < 6; ++i) {
    cpu->stage[i].busy = 1;
  }

  return cpu;
}

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
void
APEX_cpu_stop(APEX_CPU* cpu)
{
  free(cpu->code_memory);
  free(cpu);
}

/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */
int
get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

static void
print_instruction(CPU_Stage* stage)
{
  if (strcmp(stage->opcode, "STORE") == 0) {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  }

 if (strcmp(stage->opcode, "MOVC") == 0) {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
  }
  
  if (strcmp(stage->opcode, "ADD") == 0)
   {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
  if (strcmp(stage->opcode, "SUB") == 0)
   {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
  if (strcmp(stage->opcode, "MUL") == 0)
   {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
  if (strcmp(stage->opcode, "AND") == 0)
   {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
  if (strcmp(stage->opcode, "OR") == 0)
   {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
  if (strcmp(stage->opcode, "EX-OR") == 0)
   {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
  if (strcmp(stage->opcode, "LOAD") == 0)
   {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }
 if (strcmp(stage->opcode, "NOP") == 0)
   {
    printf("%s, %d\n", stage->opcode, stage->busy);
  }
   if (strcmp(stage->opcode, "BZ") == 0)
   {
    printf("%s,#%d ", stage->opcode,stage->imm);
  }
  if (strcmp(stage->opcode, "BNZ") == 0)
   {
    printf("%s,#%d ", stage->opcode,stage->imm);
  }
  if (strcmp(stage->opcode, "JUMP") == 0)
   {
    printf("%s,R%d,#%d", stage->opcode,stage->rs1, stage->imm);
  }
  if (strcmp(stage->opcode, "HALT") == 0)
   {
    printf("%s", stage->opcode);
  }
}

int 

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void
print_stage_content(char* name, CPU_Stage* stage)
{
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}

/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
	int
	fetch(APEX_CPU* cpu, int BZ_pointer)
	{
	
	CPU_Stage* stage = &cpu->stage[F];
	if (!stage->busy && !stage->stalled) { 

		/* Store current PC in fetch latch */
		stage->pc = cpu->pc;

		/* Index into code memory using this pc and copy all instruction fields into
		 * fetch latch
		 */

		if(!flag ){	
			APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
			strcpy(stage->opcode, current_ins->opcode);
			stage->rd = current_ins->rd;
			stage->rs1 = current_ins->rs1;
			stage->rs2 = current_ins->rs2;
			stage->imm = current_ins->imm;
			stage->rd = current_ins->rd;
			if (strcmp(stage->opcode, "")!=0){
				/* Update PC for next instruction */
				cpu->pc += 4;
			}
		}
		
		/* Copy data from fetch latch to decode latch*/
		if (!cpu -> stage[DRF].stalled) {
	 		cpu->stage[DRF] = cpu->stage[F];
		}
		else
		{
			stage->stalled = 1;
		}   
	}

	if (!cpu -> stage[DRF].stalled) {
 		stage->stalled = 0;
		cpu->stage[DRF] = cpu->stage[F];
 	}
				
	if (ENABLE_DEBUG_MESSAGES && BZ_pointer==0)  {
		  print_stage_content("Fetch", stage);
	}			
	return 0;
	
	}

/*
*  Decode Stage of APEX Pipeline
*
*  Note : You are free to edit this function according to your
* 				 implementation
*/
int decode(APEX_CPU* cpu, int BZ_pointer){
CPU_Stage* stage = &cpu->stage[DRF];
	

		if (!stage->busy && !stage->stalled) {
			
			if (strcmp(stage->opcode, "MOVC") == 0) {
				cpu->regs_valid[stage->rd] = 1; 
				if (!busy){
					cpu->stage[EX] = cpu->stage[DRF];
				}else {
					stage->stalled = 0;
				}
			}else if ((strcmp(stage->opcode, "BZ") == 0)||(strcmp(stage->opcode, "BNZ") == 0)){
					bz_temp++;
					if (bz_temp==1)
					{
						stage->stalled = 1;
						
						strcpy (cpu->stage[EX].opcode, "NOP");
						
					}
					if (bz_temp==2)
					{
						stage->stalled = 1;
						
						strcpy (cpu->stage[EX].opcode, "NOP");
						
					}
					if (bz_temp==3)
					{
						stage->stalled = 0;
						cpu->stage[F].stalled = 0;
						cpu->stage[EX]=cpu->stage[DRF];
						bz_temp=0;
					}
			}else if(strcmp(stage->opcode, "JUMP") == 0){
					stage->rs1_value = cpu->regs[stage->rs1];
					//cpu->regs_valid[stage->rd] = 1;
					cpu->stage[EX] = cpu->stage[DRF];
			
			}else if((strcmp(stage->opcode, "HALT") == 0))
				{
					cpu->stage[F].stalled = 1;
					cpu->stage[EX] = cpu->stage[DRF];
					
				} 
			
			
			
			else if(strcmp(stage->opcode, "LOAD") == 0){
				
				if(cpu->regs_valid[stage->rs1]==0){	
									
					stage->rs1_value = cpu->regs[stage->rs1];
					
					if (!busy) {
						cpu->stage[EX] = cpu->stage[DRF];
						
					}else {
						stage->stalled = 0;
					}
					cpu->regs_valid[stage->rd] = 1;
				}
			    else{
					stage->stalled = 1;
					
					if (!busy){
							cpu->stage[EX]=cpu->stage[DRF]; 
							cpu->stage[EX].stalled = 0;
						if (strcmp(cpu->stage[DRF].opcode, "") != 0){
							strcpy (cpu->stage[EX].opcode, "NOP");
						}
					}else {
						stage->stalled = 0;
					}
					//cpu->regs_valid[stage->rd] = 1;
						
				}
			
			
			}
			
			else if(strcmp(stage->opcode, "NOP") != 0){
				//printf("just checking\n");
				if(cpu->regs_valid[stage->rs1]==0 && cpu->regs_valid[stage->rs2]==0){			
					stage->rs1_value = cpu->regs[stage->rs1];
					stage->rs2_value = cpu->regs[stage->rs2];
					 
						cpu->stage[EX] = cpu->stage[DRF];
						cpu->stage[EX].stalled = 0;
						if(strcmp(stage->opcode, "STORE") != 0){
						cpu->regs_valid[stage->rd] = 1;
						}
				}else{
					stage->stalled = 1;
					
					if (!busy){
							cpu->stage[EX]=cpu->stage[DRF]; 
							cpu->stage[EX].stalled = 0;
						if (strcmp(cpu->stage[DRF].opcode, "") != 0){
							strcpy (cpu->stage[EX].opcode, "NOP");
						}
					}else {
						stage->stalled = 0;
					}
					//cpu->regs_valid[stage->rd] = 1;
						
				}
			}			
		}
		
		else{
			 if ((strcmp(stage->opcode, "BZ") == 0)||(strcmp(stage->opcode, "BNZ") == 0)){
				    bz_temp++;
					if (bz_temp==2)
					{
						stage->stalled = 1;
						
						strcpy (cpu->stage[EX].opcode, "NOP");
						
					}
					if (bz_temp==3)
					{
						stage->stalled = 0;
						//cpu->stage[F].stalled = 0;
						cpu->stage[EX]=cpu->stage[DRF];
						bz_temp=0;
					} 
			 }
			 else if ((strcmp(stage->opcode, "BZ") != 0)&&(strcmp(stage->opcode, "BNZ") != 0)){
					if(cpu->regs_valid[stage->rs1]==0 && cpu->regs_valid[stage->rs2]==0){	
						stage->stalled = 0;
						stage->rs1_value = cpu->regs[stage->rs1];
						stage->rs2_value = cpu->regs[stage->rs2];
										//printf("check check\n");
						if (!busy){
							cpu->stage[EX] = cpu->stage[DRF];
						}else {         
							stage->stalled = 0;
						}
						cpu->regs_valid[stage->rd] = 1;
					}  			
			}   
		}
			if (ENABLE_DEBUG_MESSAGES && BZ_pointer==0)  {
				print_stage_content("Decode/RF", stage);
			}
	return 0;
	}

/*
 *  Execute Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
 

 

int
execute(APEX_CPU* cpu, int BZ_pointer)
{
  CPU_Stage* stage = &cpu->stage[EX];

  if (!stage->busy && !stage->stalled) {
		
				if (strcmp(stage->opcode, "MUL") == 0) {	
			
			bz_temp2++;
					if (bz_temp2==1)
					{
						cpu->stage[DRF].stalled = 1;
						busy=1;
						cpu->stage[F].stalled = 1;
						//cpu->stage[MEM]=cpu->stage[EX];
						strcpy (cpu->stage[MEM].opcode, "NOP");
						
					}
					if (bz_temp2==2)
					{
						cpu->stage[DRF].stalled = 0;
						busy=0;
					    cpu->stage[F].stalled = 0;
						stage->buffer = stage->rs1_value * stage->rs2_value;
						
						cpu->stage[MEM]=cpu->stage[EX];
						bz_temp2=0;
						
					}	

		}

	
		else
		{		
			flag = 0;	
		
			/* Store */
			if (strcmp(stage->opcode, "STORE") == 0) {
				BZ_pc = stage->pc;
				stage->mem_address = stage->rs2_value + stage->imm;
				
			}
	
			if (strcmp(stage->opcode, "LOAD") == 0) {
				BZ_pc = stage->pc;
				stage->mem_address= stage->rs1_value + stage->imm;
			}
	
			if (strcmp(stage->opcode, "SUB") == 0) {
				BZ_pc = stage->pc;
				stage->buffer = stage->rs1_value - stage->rs2_value;
			}
	
			if (strcmp(stage->opcode, "AND") == 0) {
				BZ_pc = stage->pc;
				stage->buffer = stage->rs1_value & stage->rs2_value;
			}
	
			if (strcmp(stage->opcode, "OR") == 0) {
				BZ_pc = stage->pc;
				stage->buffer = stage->rs1_value | stage->rs2_value;
			}
	
			if (strcmp(stage->opcode, "ADD") == 0) {
				
				stage->buffer = stage->rs1_value + stage->rs2_value;
				
			}
	
			if (strcmp(stage->opcode, "EX-OR") == 0) {
				
				BZ_pc = stage->pc;
				stage->buffer = stage->rs1_value ^ stage->rs2_value;
			}
			/* MOVC */
			if (strcmp(stage->opcode, "MOVC") == 0) {
			}
			
			if (strcmp(stage->opcode, "JUMP") == 0) {
				stage->buffer = stage->rs1_value + stage->imm;
			}
			if (strcmp(stage->opcode, "HALT") == 0) {
				
				strcpy (cpu->stage[F].opcode, "NOP");
				strcpy (cpu->stage[DRF].opcode, "NOP");
			}
			
			if (strcmp(stage->opcode, "BZ") == 0)  {
				if (BZ_flag ==1)
				{
					Temp = stage->pc + stage->imm;
				}
			}
			if(strcmp(stage->opcode, "BNZ") == 0){
				
				if (BZ_flag==0)
				{
					Temp = stage->pc + stage->imm;
				}
			
			}
			
			cpu->stage[MEM] = cpu->stage[EX];

        }	
    }
	
	
    if (ENABLE_DEBUG_MESSAGES && BZ_pointer==0)  {
      		print_stage_content("Execute", stage);
    
  }	
  return 0;
	
}

/*
 *  Memory Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
memory(APEX_CPU* cpu, int BZ_pointer)
{
  CPU_Stage* stage = &cpu->stage[MEM];
  	
  if (!stage->busy && !stage->stalled) {
    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
		cpu->data_memory[stage->mem_address] = stage->rs1_value; 
		
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
		
    }
	if (strcmp(stage->opcode, "JUMP") == 0)
	{	
		cpu->pc = stage->buffer;
	}
	
	if (strcmp(stage->opcode, "LOAD") == 0) {	
	stage->buffer = cpu->data_memory[stage->mem_address];
	
    }
     if (strcmp(stage->opcode, "BZ") == 0)
	{
		if(BZ_flag==1)
		{
		strcpy (cpu->stage[DRF].opcode, "NOP");	
		strcpy (cpu->stage[EX].opcode, "NOP");	
		cpu->regs_valid[cpu->stage[EX].rd]=0;
		cpu->stage[F].stalled = 0;
		cpu->pc = Temp;
		BZ_flag = 0;
		}
	}
	if (strcmp(stage->opcode, "BNZ") == 0)
	{
		if(BZ_flag==0)
		{
		strcpy (cpu->stage[DRF].opcode, "NOP");	
		strcpy (cpu->stage[EX].opcode, "NOP");
		cpu->regs_valid[cpu->stage[EX].rd]=0;
		cpu->regs_valid[cpu->stage[DRF].rd]=0;
		cpu->stage[F].stalled = 0;
		cpu->pc = Temp;
		
		}
	}
	
	if (strcmp(stage->opcode, "NOP") == 0) {
		
    }
	
    /* Copy data from decode latch to execute latch*/

    	cpu->stage[WB] = cpu->stage[MEM];
  }
	if (ENABLE_DEBUG_MESSAGES && BZ_pointer==0)  {
      print_stage_content("Memory", stage);
    }
	
	
  return 0;
}

/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
writeback(APEX_CPU* cpu, int BZ_pointer)
{
	
  CPU_Stage* stage = &cpu->stage[WB];
  
  if (!stage->busy && !stage->stalled) {
    /* Update register file */
	
    if (strcmp(stage->opcode, "MOVC") == 0) {
		cpu->regs[stage->rd] = stage->imm;
		cpu->regs_valid[stage->rd] = 0;
		
        	
    }else if (strcmp(stage->opcode, "LOAD") == 0){
		cpu->regs[stage->rd] = stage->buffer;
      		cpu->regs_valid[stage->rd] = 0;
			
	}
	
	else if (strcmp(stage->opcode, "STORE") == 0)
	{
		cpu->regs_valid[stage->rd] = 0;
		
	}
	else if (strcmp(stage->opcode, "NOP") != 0 && strcmp(stage->opcode, "BZ") != 0 && strcmp(stage->opcode, "BNZ") != 0 && strcmp(stage->opcode, "JUMP") != 0 && strcmp(stage->opcode, "HALT") != 0){
		
		
		if (stage->rd != cpu->stage[MEM].rd || strcmp(cpu->stage[MEM].opcode, "NOP") == 0)
		{
		cpu->regs_valid[stage->rd] = 0;
		}
		cpu->regs[stage->rd] = stage->buffer;
		if (strcmp(stage->opcode, "ADD") == 0 || strcmp(stage->opcode, "SUB") == 0 || strcmp(stage->opcode, "MUL") == 0)
		{
		if (cpu->regs[stage->rd]==0)
		{
			BZ_flag = 1;
		}
		else{
			
		BZ_flag = 0;	
		}
		}
		
    }
	
	if (strcmp(stage->opcode, "HALT") == 0){
		Halt_flag=1;
		
	}
	
	
    if (strcmp(stage->opcode, "NOP")!= 0){
    cpu->ins_completed++;
    }
    
  }
	
    if (ENABLE_DEBUG_MESSAGES && BZ_pointer==0)  {
      print_stage_content("Writeback", stage);
    }
	
  return 0;
}

/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
char* status(int z)
{
	if (z==0)
	{
		return "Valid";
	}
	else {
		
		return "invalid";
	}
}
int
APEX_cpu_run(APEX_CPU* cpu, int x)

{
	int point=0;
  while (x>cpu->clock) {
	last_pc = ((cpu->code_memory_size-1)* 4) + 4004;
    /* All the instructions committed, so exit */
	
    

    if (ENABLE_DEBUG_MESSAGES) {
      printf("--------------------------------\n");
      printf("Clock Cycle #: %d\n", cpu->clock);
      printf("--------------------------------\n");
    }
    writeback(cpu, point);
    memory(cpu, point);
    execute(cpu, point);
    decode(cpu, point);
    fetch(cpu, point);
    cpu->clock++;
	
	 if (Halt_flag ==1){
		 printf("exiting\n");
		 break;
	 }
	
	 if (cpu->stage[WB].pc == last_pc) {
      printf("(apex) >> Simulation Complete\n");
      break;
    }
  }
  
 
	
  for (int i = 0; i < 16; i++)
         {
                printf("R%d - %d\t, status = Valid\n", i, cpu->regs[i]);
         }
  for (int i = 0; i < 32; i++)
         {
			printf("stage regs valid %d\t is %d\n",i, cpu->regs_valid[i]);
		 }
 for (int i = 0; i < 100; i++)
         {
            printf("mem address  %d\t %d\n",i, cpu->data_memory[i]);
	 }
		 
		 
		
  return 0;
}



int sim_function(APEX_CPU* cpu, int y)

{
  while (y>cpu->clock) {
int var = 1;
    /* All the instructions committed, so exit */
	last_pc = ((cpu->code_memory_size-1)* 4) + 4004;
    if (cpu->stage[WB].pc == last_pc) {
      printf("(apex) >> Simulation Complete\n");
      break;
    }
    writeback(cpu,var);
    memory(cpu,var);
    execute(cpu,var);
    decode(cpu,var);
    fetch(cpu,var);
    cpu->clock++;
  }
  for (int i = 0; i < 16; i++)
         {
                 printf("R%d - %d\t, status = Valid\n", i, cpu->regs[i]);
         }
 for (int i = 0; i < 100; i++)
         {
  printf("mem address  %d\t %d\n",i, cpu->data_memory[i]);
		 }
		
  return 0;
}
