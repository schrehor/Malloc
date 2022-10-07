#include<stdio.h>
#include<math.h>

char* memoryStart = NULL;

void memory_init(void* ptr, unsigned int size) {	 
	int numberBlocks = log2(size) - 2;
	if (size <= (numberBlocks + 5) * 4 + 8)
	{
		return;
	}
	memoryStart = ptr;

	*(int*)ptr = size;
	*((int*)ptr + 2) = numberBlocks;

	for (size_t i = 0; i < numberBlocks; i++)
	{
		*((int*)ptr + 3 + i) = 0;
	}

	for (size_t i = 0; i < size - 4 * (numberBlocks + 4); i++)
	{
		*((char*)ptr + (numberBlocks + 4) * sizeof(int) + i) = -1;
	}

	*((int*)ptr + numberBlocks + 3) = (numberBlocks + 4) * 4;
	*((int*)ptr + 1) = size - (16 + numberBlocks * 4);
}

void* memory_alloc(unsigned int size)
{
	void* start = memoryStart;
	int whereTo;
	int flag;
	int next;
	int hold;
	int biggestBlock;
	int mul = 0;

	if (size >= 50000)
	{
		biggestBlock = 50000;
	}
	else
	{
		biggestBlock = *(int*)start - (8 + 16 + *((int*)start + 2) * 4);
	}
	int smallestBlock = 8;
	int range = biggestBlock / *((int*)start + 2);

	if (size + 8 > *((int*)start + 1))
	{
		return NULL;
	}
	else if (size < smallestBlock)
	{
		return NULL;
	}
	else if (size > *((int*)start + 1) - 8)
	{
		return NULL;
	}
	else
	{
		for (size_t i = 1; i <= *((int*)start + 2); i++)	   //zistenie kam idem ukladat blok pamati
		{
			if (i == *((int*)start + 2))					   //posledny blok
			{
				if (*((int*)start + 2 + i) != 0)
				{
					next = *(int*)((char*)start + *((int*)start + 2 + i)) * -1;
					hold = *(int*)((char*)start + *((int*)start + 2 + i) + 4);
					while (next != size + mul)
					{
						if (size + mul > biggestBlock)
						{
							whereTo = *((int*)start + (*((int*)start + 2) + 3));
							flag = 0;
							break;
						}
						if (hold == 0)
						{
							mul++;
							next = *(int*)((char*)start + *((int*)start + 2 + i));
							hold = *(int*)((char*)start + *((int*)start + 2 + i) + 4);
							continue;
						}
						next = *(int*)((char*)start + hold);
						hold = *(int*)((char*)start + hold + 4);
					}
					if (size + mul > biggestBlock)
					{
						break;
					}
					else
					{
						whereTo = *(int*)((char*)start + hold);
						flag = i;
						break;
					}
				}
				else
				{
					whereTo = *((int*)start + (*((int*)start + 2) + 3));
					flag = 0;
					break;
				}
			}
			else if (range * (i - 1) <= size && size < range * i)	//zvysne bloky
			{
				if (*((int*)start + 2 + i) != 0)
				{
					next = *(int*)((char*)start + *((int*)start + 2 + i)) * -1;
					hold = *((int*)start + 2 + i);
					while (next != size + mul)
					{
						if (size + mul == range * i)
						{
							whereTo = *((int*)start + (*((int*)start + 2) + 3));
							flag = 0;
							break;
						}
						next = *(int*)((char*)start + *(int*)((char*)start + hold + 4)) * -1;
						hold = *(int*)((char*)start + hold + 4);
						if (hold == 0)
						{
							mul++;
							next = *(int*)((char*)start + *((int*)start + 2 + i)) * -1;
							hold = *((int*)start + 2 + i);
							continue;
						}
					}
					if (size + mul > range* i)
					{
						break;
					}
					else
					{
						whereTo = hold;
						flag = i;
						break;
					}
				}
				else
				{
					whereTo = *((int*)start + (*((int*)start + 2) + 3));
					flag = 0;
					break;
				}
			}
		}											  //koniec zistovania kam ulozim blok
	}

	if (flag == 0)															//ulozenie pamati do volnej pamati
	{
		*(int*)((char*)start + (*((int*)start + 2) + 3) * 4) += 8 + size;
		if (*(int*)((char*)start + (*((int*)start + 2) + 3) * 4) > *(int*)start)
		{
			*(int*)((char*)start + (*((int*)start + 2) + 3) * 4) -= (8 + size);
			return NULL;
		}
	}																		//ulozenie pamati do uz uvolneneho bloku
	else
	{
		for (size_t i = 1; i <= *((int*)start + 2); i++)
		{
			if (flag == i)
			{
				next = *((int*)start + i + 2);
				hold = next;
				while (next != whereTo)
				{
					hold = next;
					next = *(int*)((char*)start + next + 4);
				}
				if (hold == next)
				{
					next = *(int*)((char*)start + next + 4);
					*((int*)start + i + 2) = next;
					break;
				}
				else
				{
					next = *(int*)((char*)start + next + 4);
					*(int*)((char*)start + hold + 4) = next;
					break;
				}

			}
		}
	}
																	  
	*(int*)((char*)start + whereTo) = size + mul;			 //alokacia bloku
	for (size_t i = 0; i < size + mul; i++)
	{
		*((char*)start + i + whereTo + 4) = 1;
	}
	*(int*)((char*)start + whereTo + 4 + size + mul) = size + mul;		 //koniec alokacie bloku

	*((int*)start + 1) -= (size + mul + 8);				   //aktualizacia celkovej volnej pamati

	return ((char*)start + whereTo + 4);
}

int memory_free(void* valid_ptr) {
	void* current = NULL;
	void* start = memoryStart;
	int next = 0;
	int hold = 0;
	int size1,size2;
	int biggestBlock;
	(int*)current = ((int*)valid_ptr - 1);

	if (*(int*)current >= 50000)
	{
		biggestBlock = 50000;
	}
	else
	{
		biggestBlock = *(int*)start - (8 + 16 + *((int*)start + 2) * 4);
	}
	int smallestBlock = 8;
	int range = biggestBlock / *((int*)start + 2);

	// plny - uvolnit - zvysok pamati
	if (*((int*)current - 1) > 0 && (*((char*)current + 8 + *(int*)current) == -1 || *((int*)start + 1) == 0))
	{	
		size1 = *(int*)current + 8;
		*((int*)start + 3 + *((int*)start + 2)) -= size1;
		for (size_t i = 0; i < size1; i++)
		{
			*((char*)current + i) = -1;
		}
		*((int*)start + 1) += size1;

		return 0;
	}	
	// prazdny - uvolnit - zvysok pamati
	else if (*((int*)current - 1) < 0 && (*((char*)current + 8 + *(int*)current) == -1 || *((int*)start + 1) == 0))	 
	{
		size1 = *((int*)current - 1) * -1 + 8;
		size2 = size1 + *((int*)current) + 8;

		*((int*)start + 3 + *((int*)start + 2)) -= size2;		

		for (size_t i = 1; i <= *((int*)start + 2); i++)		   //odstranenie uvolneneho bloku zo zoznamu
		{
			if (i == *((int*)start + 2))						   //posledny blok
			{
				next = *((int*)start + i + 2);
				hold = next;
				while (next != (char*)current - size1 - (char*)start)
				{
					hold = next;
					next = *(int*)((char*)start + next + 4);
				}
				if (hold == next)
				{
					next = *(int*)((char*)start + next + 4);
					*((int*)start + i + 2) = next;
					break;
				}
				else
				{
					next = *(int*)((char*)start + next + 4);
					*(int*)((char*)start + hold + 4) = next;
					break;
				}
			}
			else if ((range * (i - 1) <= size1 - 8 && size1 - 8 < range * i))	   //ostatne bloky
			{
				next = *((int*)start + i + 2);
				hold = next;
				while (next != (char*)current - size1 - (char*)start)
				{
					hold = next;
					next = *(int*)((char*)start + next + 4);
				}
				if (hold == next)
				{
					next = *(int*)((char*)start + next + 4);
					*((int*)start + i + 2) = next;
					break;
				}
				else
				{
					next = *(int*)((char*)start + next + 4);
					*(int*)((char*)start + hold + 4) = next;
					break;
				}
			}
		}

		for (size_t i = 0; i < size2; i++)					   //uvolnovanie
		{
			*((char*)current - size1 + i) = -1;
		}

		*((int*)start + 1) += size2 - size1;

		return 0;
	}
	// plny - uvolnit - plny
	else if (*((int*)current - 1) > 0 && *(int*)((char*)current + *(int*)current + 8) > 0)	  
	{
		size1 = *(int*)current + 8;
		*((int*)current) *= -1;								//uvolnovanie			
		*((int*)current + 1) = 0;
		for (size_t i = 0; i < size1 - 12; i++)
		{
			*((char*)current + 8 + i) = -1;
		}
		*(int*)((char*)current + 4 + *((int*)current) * -1) *= -1;		  
		*((int*)start + 1) += size1;

		for (size_t i = 1; i <= *((int*)start + 2); i++)		 //zistenie kam zapisat offset
		{
			if (i == *((int*)start + 2))						 //posledny blok
			{
				if (*((int*)start + i + 2) == 0)
				{
					*((int*)start + i + 2) = (char*)current - (char*)start;
					break;
				}
				else
				{
					next = *((int*)start + i + 2);
					while (next != 0)
					{
						hold = next;
						next = *(int*)((char*)start + next + 4);
					}
					*(int*)((char*)start + hold + 4) = (char*)current - (char*)start;
					break;
				}
			}
			else if (range * (i - 1) <= *((int*)current)*-1 && *((int*)current)*-1 < range * i)	   //ostatne bloky
			{
				if (*((int*)start + i + 2) == 0)
				{
					*((int*)start + i + 2) = (char*)current - (char*)start;
					break;
				}
				else
				{
					next = *((int*)start + i + 2);
					while (next != 0)
					{
						hold = next;
						next = *(int*)((char*)start + next + 4);
					}
					*(int*)((char*)start + hold + 4) = (char*)current - (char*)start;
					break;
				}
			}
		}														

		return 0;
	}
	// prazdny - uvolnit - plny
	else if (*((int*)current - 1) < 0 && *(int*)((char*)current + *(int*)current + 8) > 0)
	{
		size1 = *((int*)current - 1) * -1 + 8;		
		size2 = size1 + *((int*)current) + 8;		

		*((int*)start + 1) += size2 - size1;

		*(int*)((char*)current - size1) = (size2 - 8) * -1;						 //uvolnovanie
		for (size_t i = 0; i < size2 - size1; i++)
		{
			*((char*)current + i - 4) = -1;
		}
		*(int*)((char*)current + size2 - size1 - 4) = (size2 - 8) * -1;



		for (size_t i = 1; i <= *((int*)start + 2); i++)				   //odstranenie bloku zo zoznamu
		{
			if (i == *((int*)start + 2))								   //posledny blok
			{
				next = *((int*)start + i + 2);
				hold = next;
				while (next != (char*)current - size1 - (char*)start)
				{
					hold = next;
					next = *(int*)((char*)start + next + 4);
				}
				if (hold == next)
				{
					next = *(int*)((char*)start + next + 4);
					*((int*)start + i + 2) = next;
					break;
				}
				else
				{
					next = *(int*)((char*)start + next + 4);
					*(int*)((char*)start + hold + 4) = next;
					break;
				}
			}
			else if ((range * (i - 1) <= size1 - 8 && size1 - 8 < range * i))	   //ostatne bloky
			{
				next = *((int*)start + i + 2);
				hold = next;
				while (next != (char*)current - size1 - (char*)start)
				{
					hold = next;
					next = *(int*)((char*)start + next + 4);
				}
				if (hold == next)
				{
					next = *(int*)((char*)start + next + 4);
					*((int*)start + i + 2) = next;
					break;
				}
				else
				{
					next = *(int*)((char*)start + next + 4);
					*(int*)((char*)start + hold + 4) = next;
					break;
				}
			}
		}

		(char*)current = ((char*)current - size1);
		*((int*)current + 1) = 0;

		for (size_t i = 1; i <= *((int*)start + 2); i++)		 //zistenie kam zapisat offset
		{
			if (i == *((int*)start + 2))							 //posledny blok
			{
				if (*((int*)start + i + 2) == 0)
				{
					*((int*)start + i + 2) = (char*)current - (char*)start;
					break;
				}
				else
				{
					next = *((int*)start + i + 2);
					while (next != 0)
					{
						hold = next;
						next = *(int*)((char*)start + next + 4);
					}
					*(int*)((char*)start + hold + 4) = (char*)current - (char*)start;
					break;
				}
			}
			else if (range * (i - 1) <= size2 - 8 && size2 - 8 < range * i)		   //ostatne bloky
			{
				if (*((int*)start + i + 2) == 0)
				{
					*((int*)start + i + 2) = (char*)current - (char*)start;
					break;
				}
				else
				{
					next = *((int*)start + i + 2);
					while (next != 0)
					{
						hold = next;
						next = *(int*)((char*)start + next + 4);
					}
					*(int*)((char*)start + hold + 4) = (char*)current - (char*)start;
					break;
				}
			}
		}														

		return 0;
	}
	// prazdny - uvolnit - prazdny
	else if (*((int*)current - 1) < 0 && *(int*)((char*)current + *(int*)current + 8) < 0)
	{
		size1 = *((int*)current - 1) * -1 + 8;
		size2 = size1 + *((int*)current) + 8;
		
		*((int*)start + 1) += size2 - size1;

		*(int*)((char*)current - size1) = (size2 - 8) * -1;					   //spojenie
		for (size_t i = 0; i < size2 - size1; i++)
		{
			*((char*)current + i - 4) = -1;
		}
		*(int*)((char*)current + size2 - size1 - 4) = (size2 - 8) * -1;

		for (size_t i = 1; i <= *((int*)start + 2); i++)				//odkial je prazdny pred uvolnenym
		{
			if (i == *((int*)start + 2))							    //posledny blok
			{
				next = *((int*)start + i + 2);
				hold = next;
				while (next != (char*)current - size1 - (char*)start)
				{
					hold = next;
					next = *(int*)((char*)start + next + 4);
				}
				if (hold == next)
				{
					next = *(int*)((char*)start + next + 4);
					*((int*)start + i + 2) = next;
					break;
				}
				else
				{
					next = *(int*)((char*)start + next + 4);
					*(int*)((char*)start + hold + 4) = next;
					break;
				}
			}
			else if ((range * (i - 1) <= size1 - 8 && size1 - 8 < range * i))		  //ostatne bloky
			{
				next = *((int*)start + i + 2);
				hold = next;
				while (next != (char*)current - size1 - (char*)start)
				{
					hold = next;
					next = *(int*)((char*)start + next + 4);
				}
				if (hold == next)
				{
					next = *(int*)((char*)start + next + 4);
					*((int*)start + i + 2) = next;
					break;
				}
				else
				{
					next = *(int*)((char*)start + next + 4);
					*(int*)((char*)start + hold + 4) = next;
					break;
				}
			}
		}

		*(int*)((char*)current - size1 + 4) = 0;
			   
		(char*)current = ((char*)current + size2 - size1);
		size1 = *(int*)current * -1 + 8;
		size2 += size1;

		for (size_t i = 1; i <= *((int*)start + 2); i++)			//odkial je prazdny po uvolnenom
		{
			if (i == *((int*)start + 2))						    //posledny blok
			{
				next = *((int*)start + i + 2);
				hold = next;
				while (next != (char*)current - (char*)start)
				{
					hold = next;
					next = *(int*)((char*)start + next + 4);
				}
				if (hold == next)
				{
					next = *(int*)((char*)start + next + 4);
					*((int*)start + i + 2) = next;
					break;
				}
				else
				{
					next = *(int*)((char*)start + next + 4);
					*(int*)((char*)start + hold + 4) = next;
					break;
				}
			}
			else if ((range * (i - 1) <= size1 - 8 && size1 - 8 < range * i))	   //ostatne bloky
			{
				next = *((int*)start + i + 2);
				hold = next;
				while (next != (char*)current - (char*)start)
				{
					hold = next;
					next = *(int*)((char*)start + next + 4);
				}
				if (hold == next)
				{
					next = *(int*)((char*)start + next + 4);
					*((int*)start + i + 2) = next;
					break;
				}
				else
				{
					next = *(int*)((char*)start + next + 4);
					*(int*)((char*)start + hold + 4) = next;
					break;
				}
			}
		}

		*(int*)((char*)current - size2 + size1) = (size2 - 8) * -1;		   //spojenie
		for (size_t i = 0; i < size1; i++)
		{
			*((char*)current + i - 4) = -1;
		}
		*(int*)((char*)current + size1 - 4) = (size2 - 8) * -1;

		(char*)current = ((char*)current - size2 + size1);

		for (size_t i = 1; i <= *((int*)start + 2); i++)		 //zistenie kam zapisat offset
		{
			if (i == *((int*)start + 2))						 //posledny blok
			{
				if (*((int*)start + i + 2) == 0)
				{
					*((int*)start + i + 2) = (char*)current - (char*)start;
					break;
				}
				else
				{
					next = *((int*)start + i + 2);
					while (next != 0)
					{
						hold = next;
						next = *(int*)((char*)start + next + 4);
					}
					*(int*)((char*)start + hold + 4) = (char*)current - (char*)start;
					break;
				}
			}
			else if (range * (i - 1) <= size2 - 8 && size2 - 8 < range * i)			  //ostatne bloky
			{
				if (*((int*)start + i + 2) == 0)
				{
					*((int*)start + i + 2) = (char*)current - (char*)start;
					break;
				}
				else
				{
					next = *((int*)start + i + 2);
					while (next != 0)
					{
						hold = next;
						next = *(int*)((char*)start + next + 4);
					}
					*(int*)((char*)start + hold + 4) = (char*)current - (char*)start;
					break;
				}
			}
		}

		return 0;
	}
	//plny - uvolnit - prazdny
	else if (*((int*)current - 1) > 0 && *(int*)((char*)current + *(int*)current + 8) < 0)
	{
		size1 = *(int*)current  + 8;
		size2 = size1 + 8 + *(int*)((char*)current + size1) * -1;

		(char*)current = ((char*)current + size1);

		for (size_t i = 1; i <= *((int*)start + 2); i++)			  //odstranenie bloku zo zoznamu
		{
			if (i == *((int*)start + 2))						   //posledny blok
			{
				next = *((int*)start + i + 2);
				hold = next;
				while (next != (char*)current - (char*)start)
				{
					hold = next;
					next = *(int*)((char*)start + next + 4);
				}
				if (hold == next)
				{
					next = *(int*)((char*)start + next + 4);
					*((int*)start + i + 2) = next;
					break;
				}
				else
				{
					next = *(int*)((char*)start + next + 4);
					*(int*)((char*)start + hold + 4) = next;
					break;
				}
			}
			else if ((range * (i - 1) <= size1 - 8 && size1 - 8 < range * i))		 //ostatne bloky
			{
				next = *((int*)start + i + 2);
				hold = next;
				while (next != (char*)current - (char*)start)
				{
					hold = next;
					next = *(int*)((char*)start + next + 4);
				}
				if (hold == next)
				{
					next = *(int*)((char*)start + next + 4);
					*((int*)start + i + 2) = next;
					break;
				}
				else
				{
					next = *(int*)((char*)start + next + 4);
					*(int*)((char*)start + hold + 4) = next;
					break;
				}
			}
		}

		*((int*)current + 1) = 0;

		(char*)current = ((char*)current - size1);
		*(int*)current = size2 * -1;								   //spojenie
		*((int*)current + 1) = 0;
		for (size_t i = 0; i < size1; i++)
		{
			*((char*)current + i + 8) = -1;
		}
		*(int*)((char*)current + size2 - 4) = size2 * -1;

		for (size_t i = 1; i <= *((int*)start + 2); i++)		 //zistenie kam zapisat offset
		{
			if (i == *((int*)start + 2))								 //posledny blok
			{
				if (*((int*)start + i + 2) == 0)
				{
					*((int*)start + i + 2) = (char*)current - (char*)start;
					break;
				}
				else
				{
					next = *((int*)start + i + 2);
					while (next != 0)
					{
						hold = next;
						next = *(int*)((char*)start + next + 4);
					}
					*(int*)((char*)start + hold + 4) = (char*)current - (char*)start;
					break;
				}
			}
			else if (range * (i - 1) <= size2 - 8 && size2 - 8 < range * i)				  //ostatne bloky
			{
				if (*((int*)start + i + 2) == 0)
				{
					*((int*)start + i + 2) = (char*)current - (char*)start;
					break;
				}
				else
				{
					next = *((int*)start + i + 2);
					while (next != 0)
					{
						hold = next;
						next = *(int*)((char*)start + next + 4);
					}
					*(int*)((char*)start + hold + 4) = (char*)current - (char*)start;
					break;
				}
			}
		}
	}
}

int memory_check(void* ptr) {
	void* current = NULL;
	void* start = memoryStart;

	(int*)current = ((int*)ptr - 1);

	if (*(int*)current >= 8 && *((char*)current + 4) == 1)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void test1() {
	char memory[50];
	int sizeMemory = 50;
	int i = 0, y = 0, alok = 8, alok1 = 0, alok2 = 0;
	double result;

	memory_init(memory, sizeMemory);

	while (sizeMemory > y* alok)
	{
		void* a = memory_alloc(alok);
		if (a != NULL) {
			i++;
			alok1 += alok;
		}
		y++;
		alok2 += alok;
		alok = rand() % 16 + 8;
	}

	result = (double)alok1 / (double)alok2 * 100;

	printf("Celkova pamat: %d, Alokovanych bytov: %d, Percentualna uspesnost oproti idealnemu rieseniu: %.2lf\n", sizeMemory, alok, result);

	char memory1[100];
	sizeMemory = 100;
	i = 0, y = 0, alok = 12, alok1 = 0, alok2 = 0;

	memory_init(memory1, sizeMemory);

	while (sizeMemory > y* alok)
	{
		void* a = memory_alloc(alok);
		if (a != NULL) {
			i++;
			alok1 += alok;
		}
		y++;
		alok2 += alok;
		alok = rand() % 16 + 8;
	}

	result = (double)alok1 / (double)alok2 * 100;

	printf("Celkova pamat: %d, Alokovanych bytov: %d, Percentualna uspesnost oproti idealnemu rieseniu: %.2lf\n", sizeMemory, alok, result);

	char memory2[200];
	sizeMemory = 200;
	i = 0, y = 0, alok = 24, alok1 = 0, alok2 = 0;

	memory_init(memory2, sizeMemory);

	while (sizeMemory > y* alok)
	{
		void* a = memory_alloc(alok);
		if (a != NULL) {
			i++;
			alok1 += alok;
		}
		y++;
		alok2 += alok;
		alok = rand() % 16 + 8;
	}

	result = (double)alok1 / (double)alok2 * 100;

	printf("Celkova pamat: %d, Alokovanych bytov: %d, Percentualna uspesnost oproti idealnemu rieseniu: %.2lf\n", sizeMemory, alok, result);
}

void test2() {
	char memory[200];
	int sizeMemory = 200;
	int i = 0, y = 0, alok = rand() % 16 + 8, alok1 = 0, alok2 = 0;
	double result;

	memory_init(memory, sizeMemory);

	while (sizeMemory > alok2)
	{
		void* a = memory_alloc(alok);
		if (a != NULL) {
			i++;
			alok1 += alok;
		}
		y++;
		alok2 += alok;
		alok = rand() % 16 + 8;
	}

	result = (double)alok1 / (double)alok2 * 100;

	printf("Celkova pamat: %d, Alokovanie nahodnych bytov od 8 - 24, Percentualna uspesnost oproti idealnemu rieseniu: %.2lf\n", sizeMemory, result);

}

void test3() {
	char memory[500000];
	int sizeMemory = 500000;
	int i = 0, y = 0, alok = rand() % 4500 + 500, alok1 = 0, alok2 = 0;
	double result;

	memory_init(memory, sizeMemory);

	while (sizeMemory > alok2)
	{
		void* a = memory_alloc(alok);
		if (a != NULL) {
			i++;
			alok1 += alok;
		}
		y++;
		alok2 += alok;
		alok = rand() % 4500 + 500;
	}

	result = (double)alok1 / (double)alok2 * 100;

	printf("Celkova pamat: %d, Alokovanie nahodnych bytov od 500 - 5000, Percentualna uspesnost oproti idealnemu rieseniu: %.2lf\n", sizeMemory, result);
}

void test4() {
	char memory[500000];
	int sizeMemory = 500000;
	int i = 0, y = 0, alok = rand() % 49992 + 8, alok1 = 0, alok2 = 0;
	double result;

	memory_init(memory, sizeMemory);

	while (sizeMemory > alok2)
	{
		void* a = memory_alloc(alok);
		if (a != NULL) {
			i++;
			alok1 += alok;
		}
		y++;
		alok2 += alok;
		alok = rand() % 49992 + 8;
	}

	result = (double)alok1 / (double)alok2 * 100;

	printf("Celkova pamat: %d, Alokovanie nahodnych bytov od 8 - 50000, Percentualna uspesnost oproti idealnemu rieseniu: %.2lf\n", sizeMemory, result);

}

int main() {
	test1();
	test2();
	test3();
	test4();

	return 0;
}