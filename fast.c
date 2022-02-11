
#include <stdlib.h>
#include <stdio.h>

#include "j1939.h"
#include "fast.h"

int fast = 0;
int id = 0xff;
int seq;
int left = 0;

int is_fast(int can_id)
{
	switch (PGN(can_id))
	{
		case 127489:
		case 126464:
		case 126208:
		case 129795:
		case 129794:
		case 129797:
		case 129029:
		case 129285:
		case 127237:
		case 129540:
		case 129796:
		case 129284:
		case 127497:
		case 127496:
		case 127498:
		case 129039:
		case 127503:
		case 129038:
		case 129041:
		case 129809:
		case 130320:
		case 129040:
		case 128275:
		case 130578:
		case 129810:
		case 129045:
		case 129044:
		case 126996:
		case 126998:
			return 1;
		default:
			return 0;
	}
}
void fastprefix(int canId, int b, int c)
{
	int s;
	int i;

	s = b & 0x1f;
	i = b & 0xe0;
	if (i != id)
	{
		if (s == 0)
		{
			fast = canId;
			id = i;
			seq = 0;
			left = c - 6;
		}
		else
		{
			fast = 0;
			id = 0xff;
		}
	}
	else
	{
		if (fast == canId)
		{
			left = left - 7;
			if (s != seq + 1)
			{
				if (seq > 2 && left > 0)
				{
					fprintf(stderr, "missing packet left=%d\n", left);
				}
				fast = 0;
				id = 0xff;
			}
			seq = s;
		}
	}
}
