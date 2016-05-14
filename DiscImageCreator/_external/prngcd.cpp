//cd scramble as explained by http://www.ecma-international.org/publications/files/ECMA-ST/Ecma-130.pdf
/*Copyright (c) 2015 Jonathan Gevaryahu
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// #include "stdio.h"
unsigned char scrambled_table[2352];

// int main()
void make_scrambled_table(void)
{
	long counter = 0x0001; //counter starts at 0001
	char newval = 0;
	unsigned long outval = 0;
	int loopcounter;
	// output exactly 12 zeroes first, which corresponds to the sync portion of the sector, see ECMA-130
	for (loopcounter=0; loopcounter < 12; loopcounter++)
//		printf("0x00 ");
		scrambled_table[loopcounter] = 0;
	int newcounter = loopcounter;
	// now the actual scramble lfsr:
	for (loopcounter=0; loopcounter < 2340*8; loopcounter++)
	{
		newval = (((counter&1)?1:0)^((counter&2)?1:0));
		//fprintf(stderr,"%d",counter&1);
		outval |= (counter&1)?0x8000:0; // outval is 16 bits, counter is only 15 bits!
		if ((loopcounter%16)==15)
		{
			//fprintf(stderr, " 0x%04x\n",outval);
//			printf("0x%02x ", outval&0xFF);
//			printf("0x%02x ", (outval>>8)&0xFF);
			scrambled_table[newcounter++] = outval & 0xFF;
			scrambled_table[newcounter++] = (outval >> 8) & 0xFF;
		}
		outval >>= 1;
		counter >>= 1;
		if (newval == 1) counter |= 0x4000; // ECMA-130 diagram is wrong and shows this as counter |= 0x8000; the text is correct.
	}
}
