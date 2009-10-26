/*
 * Milkymist Emulation Libraries
 * Copyright (C) 2007, 2008, 2009 Sebastien Bourdeauducq
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <hal/pfpu.h>

void pfpu_init()
{
	printf("FPU: software-based emulation\n");
}

void pfpu_isr()
{
	printf("pfpu_isr() should not be called in emulation mode\n");
}

struct pfpu_pipe {
	union {
		float f;
		int i;
	} result;
	int valid;
};

struct pfpu_pipeset {
	struct pfpu_pipe pipe5[5];
	struct pfpu_pipe pipe4[4];
	struct pfpu_pipe pipe3[3];
	struct pfpu_pipe pipe2[2];
};

static float i2f(int i)
{
	union {
		float _f;
		int _i;
	} conv;
	conv._i = i;
	return conv._f;
}

static int f2i(float f)
{
	union {
		float _f;
		int _i;
	} conv;
	conv._f = f;
	return conv._i;
}

static void pfpu_issue(struct pfpu_pipeset *pipeset, int opa, int opb, int opcode)
{
	switch(opcode) {
		case PFPU_OPCODE_NOP:
			break;
		case PFPU_OPCODE_FADD:
			pipeset->pipe4[3].valid = 1;
			pipeset->pipe4[3].result.f = i2f(opa)+i2f(opb);
			break;
		case PFPU_OPCODE_FSUB:
			pipeset->pipe4[3].valid = 1;
			pipeset->pipe4[3].result.f = i2f(opa)-i2f(opb);
			break;
		case PFPU_OPCODE_FMUL:
			pipeset->pipe5[4].valid = 1;
			pipeset->pipe5[4].result.f = i2f(opa)*i2f(opb);
			break;
		case PFPU_OPCODE_F2I:
			pipeset->pipe2[1].valid = 1;
			pipeset->pipe2[1].result.i = i2f(opa);
			break;
		case PFPU_OPCODE_I2F:
			pipeset->pipe3[2].valid = 1;
			pipeset->pipe3[2].result.f = opa;
			break;
		case PFPU_OPCODE_VECT:
			pipeset->pipe2[1].valid = 1;
			pipeset->pipe2[1].result.i = opa | (opb << 16);
			break;
		case PFPU_OPCODE_SIN:
			pipeset->pipe4[3].valid = 1;
			pipeset->pipe4[3].result.f = sinf((float)opa/PFPU_TRIG_CONV);
			break;
		case PFPU_OPCODE_COS:
			pipeset->pipe4[3].valid = 1;
			pipeset->pipe4[3].result.f = cosf((float)opa/PFPU_TRIG_CONV);
			break;
		case PFPU_OPCODE_ABOVE:
			pipeset->pipe2[1].valid = 1;
			pipeset->pipe2[1].result.f = i2f(opa) > i2f(opb) ? 1.0f : 0.0f;
			break;
		case PFPU_OPCODE_EQUAL:
			pipeset->pipe2[1].valid = 1;
			pipeset->pipe2[1].result.f = i2f(opa) == i2f(opb) ? 1.0f : 0.0f;
			break;
		case PFPU_OPCODE_COPY:
			pipeset->pipe2[1].valid = 1;
			pipeset->pipe2[1].result.i = opa;
			break;
		default:
			printf("FPU: unknown opcode 0x%02x\n", opcode);
			break;
	}
}

static void pfpu_process(struct pfpu_pipeset *pipeset, int *wcount, int *val)
{
	memmove(&pipeset->pipe5[0], &pipeset->pipe5[1], 4*sizeof(struct pfpu_pipe));
	memmove(&pipeset->pipe4[0], &pipeset->pipe4[1], 3*sizeof(struct pfpu_pipe));
	memmove(&pipeset->pipe3[0], &pipeset->pipe3[1], 2*sizeof(struct pfpu_pipe));
	memmove(&pipeset->pipe2[0], &pipeset->pipe2[1], 1*sizeof(struct pfpu_pipe));
	pipeset->pipe5[4].valid = 0;
	pipeset->pipe4[3].valid = 0;
	pipeset->pipe3[2].valid = 0;
	pipeset->pipe2[1].valid = 0;
	*wcount = 0;
	if(pipeset->pipe5[0].valid) { *val = pipeset->pipe5[0].result.i; (*wcount)++; }
	if(pipeset->pipe4[0].valid) { *val = pipeset->pipe4[0].result.i; (*wcount)++; }
	if(pipeset->pipe3[0].valid) { *val = pipeset->pipe3[0].result.i; (*wcount)++; }
	if(pipeset->pipe2[0].valid) { *val = pipeset->pipe2[0].result.i; (*wcount)++; }
}

static int pfpu_exec(struct pfpu_td *td, int x, int y, unsigned int *output)
{
	int i, j;
	struct pfpu_pipeset pipeset;
	int wcount;
	int regf[PFPU_REG_COUNT];
	
	for(i=0;i<PFPU_REG_COUNT;i++)
		regf[i] = f2i(td->registers[i]);
	for(i=0;i<PFPU_PROGSIZE;i++) {
		regf[0] = x;
		regf[1] = y;
		pfpu_issue(&pipeset, regf[td->program[i].i.opa], regf[td->program[i].i.opb], td->program[i].i.opcode);
		pfpu_process(&pipeset, &wcount, &regf[td->program[i].i.dest]);
		if(wcount > 1)
			printf("FPU: collision at cycle %d\n", i);
		if((wcount == 0) && (td->program[i].i.dest != 0))
			printf("FPU: no result to write to register at cycle %d\n", i);
		if((wcount != 0) && (td->program[i].i.dest == 0))
			printf("FPU: no register to write result at cycle %d (stray write)\n", i);
		if(td->program[i].i.dest == PFPU_REG_OUT) {
			*output = regf[PFPU_REG_OUT];
			if(td->update) {
				for(j=0;j<PFPU_REG_COUNT;j++)
					td->registers[j] = i2f(regf[j]);
			}
			return 1;
		}
	}
	
	return 0;
}

int pfpu_submit_task(struct pfpu_td *td)
{
	int x, y;
	
	for(y=0;y<td->vmeshlast;y++)
		for(x=0;x<td->hmeshlast;x++) {
			if(!pfpu_exec(td, x, y, td->output+128*y+x)) {
				printf("FPU: program did not terminate, calling completion callback anyway\n");
				printf("FPU: this would lockup the real hardware\n");
			}
		}
	
	td->callback(td);
	
	return 1;
}
