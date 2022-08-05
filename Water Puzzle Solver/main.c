#define _CRT_SECURE_NO_WARNINGS
#define LFG_K 10
#define LFG_J 7
#define LFG_MOD RAND_MAX
#define BOTTLE_DEPTH 4
#define RESET printf("\033[0m")
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h> //for memset

unsigned int state[10]; //array of states needed for the LFG random number generator

typedef enum { GAME_NOT_FINISHED, GAME_WIN, GAME_LOSE } winState;

//STRUCTURES
typedef struct bottle {
	char colors[BOTTLE_DEPTH];
	char length;// current number of colors in the bottle, defaults to 0
}bottle;

typedef struct gameState {
	bottle* bottles;
	winState win;
} gameState;

typedef struct node {
	char iz_bocice;
	char u_bocicu;
	char depth;
	gameState state;
	winState win;
	struct node* child, * brother, * father;
	char isWinPath;
} Node;

typedef struct Tree {
	Node* root;
	unsigned int size;
} Tree;

typedef struct qnode {
	struct qnode* next;
	Node* val; 
} qNode;

typedef struct {
	struct qnode* head;
	struct qnode* tail;
} queue;

typedef struct snode {
	struct snode* prev;
	Node* val;
} sNode;

typedef struct {
	sNode* top;
} stack;
//END STRUCTURES

//QUEUE FUNCTIONS
void initQueue(queue* q) {
	q->head = NULL;
	q->tail = NULL;
}

int queueIsEmpty(queue* q) {
	return (q->head == NULL);
}

int enqueue(queue* q, Node* val) {
	qNode* tmp = (qNode*)malloc(sizeof(qNode));
	if (tmp == NULL) return 0;
	tmp->val = val;
	tmp->next = NULL;
	if (q->tail != NULL) q->tail->next = tmp;
	q->tail = tmp;
	if (q->head == NULL) q->head = tmp;
	return 1;
}

Node* dequeue(queue* q) {
	//dont check if it's empty, already considered in the main program
	qNode* tmp = q->head;
	Node* val = tmp->val;
	q->head = q->head->next;
	if (q->head == NULL) q->tail = NULL;
	free(tmp);
	return val;
}
//END_QUEUE FUNCTIONS

//STACK FUNCTIONS
void initStack(stack* s) {
	s->top = NULL;
}

int stackIsEmpty(stack* s) {
	return (s->top == NULL);
}

int push(stack* s, Node* val) {
	sNode* tmp = (sNode*)malloc(sizeof(sNode));
	if (tmp == NULL) return 0;
	tmp->val = val;
	tmp->prev = s->top;
	s->top = tmp;
	return 1;
}

Node* pop(stack* s) {
	if (s->top == NULL) return 0;
	sNode* tmp = s->top;
	Node* val;
	s->top = tmp->prev;
	val = tmp->val;
	free(tmp);
	return val;
}
//END STACK FUNCTIONS

//PRINTING FUNCTIONS
void printColor(int colorCode) {
	switch (colorCode)
	{
	case 0: printf("\033[37;41m"); break; //red
	case 1:printf("\033[37;42m"); break;//green
	case 2: printf("\033[37;43m"); break;//yellow
	case 3: printf("\033[37;44m"); break;//blue
	case 4: printf("\033[37;45m"); break;//magenta
	case 5: printf("\033[37;46m"); break;//cyan
	case 6: printf("\033[37;47m"); break;//white
	case 7: printf("\033[101m"); break;// bright red
	case 8: printf("\033[102m"); break;// bright green
	case 9: printf("\033[103m"); break;// bright yellow
	case 10: printf("\033[104m"); break;// bright blue
	case 11: printf("\033[105m"); break;// bright magenta
	case 12: printf("\033[106m"); break;// bright cyan
	default: printf("%d", colorCode);  break;
	}
}

void printGameState(gameState game, int n) {
	printf("\n");
	for (int i = 0; i < n; i++) {
		printf("\t  #%d    ", i);
	}
	printf("\n\n");
	for (int j = BOTTLE_DEPTH - 1; j >= 0; j--)
	{
		for (int i = 0; i < n; i++)
		{
			if (j >= game.bottles[i].length) {
				printf("\t|   |\t");
			}
			else {
				printf("\t|"); printColor((int)game.bottles[i].colors[j]); printf("   "); RESET;  printf("|\t");
			}
		}
		printf("\n");
	}
	printf("\n\n");
}

void printDepth(char depth) {
	for (char i = 0; i < depth; i++) printf("-");
}

void printBottlesOnLine(gameState state, int n) {
	for (int i = 0; i < n; i++) {
		printf("{");
		for (int j = 0; j < BOTTLE_DEPTH - 1; j++)
		{
			if (j >= state.bottles[i].length)  printf("-, ");
			else printf("%d, ", state.bottles[i].colors[j]);
		}
		if (state.bottles[i].length == BOTTLE_DEPTH)
			printf("%d", state.bottles[i].colors[BOTTLE_DEPTH - 1]);
		else printf("-");
		printf("}; ");
	}
	printf("\n");
}

void printLevelNode(Node* parent, Node* child, int n) {
	printColor(child->depth);
	printf("  ");
	RESET;
	printf("==============DEPTH: %d===============", child->depth);
	printColor(child->depth);
	printf("  ");
	RESET;
	printf("\n");
	printDepth(child->depth);
	printf("FROM BOTTLE %d TO BOTTLE %d\n", child->iz_bocice, child->u_bocicu);
	if (parent != NULL) {
		printDepth(child->depth);
		printf("PARENT BOTTLES: ");
		printBottlesOnLine(parent->state, n);
	}
	printDepth(child->depth);
	printf("BOTTLES: ");
	printBottlesOnLine(child->state, n);
	printDepth(child->depth);
	printf("GAME STATE: ");
	if (child->win == GAME_NOT_FINISHED) {
		printf("NOT FINISHED\n");
	}
	else if (child->win == GAME_WIN) {
		printf("WIN\n");
	}
	else printf("LOSE\n");
	printf("\n");
}

void printSolutionSteps(int* steps, int nrSolvingSteps) {
	printf("~~~~~~~~ Number of steps: %d ~~~~~~~~~~\n", nrSolvingSteps / 2);
	for (int i = 0; i < nrSolvingSteps - 3; i += 2)
	{
		printf("(%d, %d) , ", steps[i], steps[i + 1]);
	}
	printf("(%d, %d)\n", steps[nrSolvingSteps - 2], steps[nrSolvingSteps - 1]);
}

void printPreorder(Node* root, int n) {
	stack s;
	initStack(&s);
	Node* curr, * brother, * parent = NULL;
	push(&s, root);
	while (!stackIsEmpty(&s)) {
		curr = pop(&s);
		while (curr != NULL) {
			printLevelNode(parent, curr, n);
			brother = curr->brother;
			while (brother != NULL) {
				push(&s, brother);
				brother = brother->brother;
			}
			parent = curr;
			curr = curr->child;
		}
	}
}
//END PRINTING FUNCTIONS

//PRNG FUNCTIONS
int initLFGstate() {
	int evenCnt = 0;
	srand(time(NULL));
	for (int i = 0; i < LFG_K; i++) state[i] = rand();
	for (int i = 0; i < LFG_K; i++) {
		if (state[i] % 2 == 0) evenCnt++;
	}
	if (evenCnt == LFG_K) return 0; // LFG doesn't work if all states are even numbers
	return 1;
}

int getNextRand() {
	unsigned int out;
	out = (state[LFG_J - 1] + state[LFG_K - 1]) % LFG_MOD;
	for (int i = 0; i < LFG_K - 1; i++) {
		state[i] = state[i + 1];
	}
	state[LFG_K - 1] = out;
	return out;
}
//END PRNG FUNCTIONS

//BOTTLE FUNCTIONS
int getTopColor(bottle b) {

	if (b.length == 0)
	{
		printf("Null in getTopColor");
		exit(-1);
	}
	return b.colors[b.length - 1];
}

void setTopColor(bottle* b, int color) {
	b->colors[b->length] = color;
}

int canMoveBetweenBottles(bottle* b1, bottle* b2) {
	if (b1 == NULL || b2 == NULL) return 0;
	if (b2->length == BOTTLE_DEPTH || b1->length == 0) return 0;
	int color1, color2;
	color1 = getTopColor(*b1);
	if (b2->length == 0) color2 = -1;
	else color2 = getTopColor(*b2);
	if (color1 != color2 && b2->length != 0) return 0;
	return 1;
}

int moveBetweenBottles(bottle* b1, bottle* b2) {
	//move color form top of b1 to top of b2
	if (b1 == NULL || b2 == NULL)
	{
		printf("Null in moveBetweenBottles");
		return 0;
	}
	if (b1->length == 0) return 0;
	int color1 = getTopColor((*b1));
	int color2 = (b2->length == 0) ? -1 : getTopColor(*b2);
	int nrMoved = 0;
	if (b1->length == 0) return 0;
	while (b2->length == 0 || (color1 == color2 && b2->length != BOTTLE_DEPTH && b2->length > 0)) {
		//printf("#moveBetweenBottles\n"); DEBUG
		setTopColor(b2, color1);
		b2->length++;
		b1->length--;
		color2 = color1;
		if (b1->length == 0) break;
		color1 = getTopColor((*b1));
		nrMoved++;
	}

	return nrMoved;
}

int bottleHasSameColors(bottle b) {//checks if bottle is complete
	if (b.length == BOTTLE_DEPTH) {
		for (int i = 0; i < BOTTLE_DEPTH - 1; i++)
		{
			if (b.colors[i] == b.colors[i + 1]) continue;
			return 0;
		}
		return 1;
	}
	return 0;
}
//END BOTTLE FUNCTIONS

// HELPER COPY FUNCTION
gameState deepCopy(gameState game, int n, int k) {
	gameState newGame;
	newGame.bottles = (bottle*)calloc(n, sizeof(bottle));
	newGame.win = game.win;
	for (int i = 0; i < n; i++) {
		newGame.bottles[i].length = game.bottles[i].length;
		for (int j = 0; j < BOTTLE_DEPTH; j++)
		{
			newGame.bottles[i].colors[j] = game.bottles[i].colors[j];
		}
	}
	return newGame;
}
//END HELPER COPY FUNCTION

//MAIN GAME LOGIC FUNCTIONS
winState gameOver(gameState game, int n, int k, int p) {
	int cnt = 0;
	if (p == 0) return GAME_LOSE;
	for (int i = 0; i < n; i++)
	{
		if (bottleHasSameColors(game.bottles[i]))
		{
			cnt++;
		}
	}
	if (cnt == n - k) return GAME_WIN;
	return GAME_NOT_FINISHED;
}

int getSelectedMenuOption() {
	int selectedOption;
	printf("\t1) Generate start state\n\
	2) Play a move\n\
	3) Hint\n\
	4) Print game tree (Preorder) (Note: Might take ages to print)\n\
	5) Print one game solution\n\
	6) Free memory\n\
	7) Generate test game state (3 bottles, 1 empty, 31 nodes in game tree) \n\
	0) Exit program\n");
	printf("Input: ");
	while (!scanf("%d", &selectedOption) || selectedOption < 0 || selectedOption > 7) {
		printf("Invalid input, try again or input 0 to exit the program.\n");
		printf("Input: ");
	}
	return selectedOption;
}

Node* initializeNode() {
	Node* t;
	t = (Node*)malloc(sizeof(Node));
	if (t == NULL) return NULL;
	t->child = NULL;
	t->brother = NULL;
	return t;
}

Node* addChildNode(Node* father, int iz_bocice, int u_bocicu, gameState state, winState win) {//dodaj informacioni sadrzaj
	Node* child = initializeNode(), * tmp;
	if (child == NULL) return NULL;
	child->father = father;
	child->depth = father->depth + 1;
	child->iz_bocice = iz_bocice;
	child->u_bocicu = u_bocicu;
	child->state = state;
	child->win = win;
	child->isWinPath = 0;
	tmp = father->child;
	if (tmp == NULL) father->child = child;
	else {
		while (tmp->brother != NULL) tmp = tmp->brother;
		tmp->brother = child;
	}
	return child;
}

void freeSubtree(Node* n) {
	if (n == NULL) return;
	queue q;
	gameState state;
	initQueue(&q);
	enqueue(&q, n);
	while (!queueIsEmpty(&q)) {
		Node* curr = dequeue(&q);
		Node* child = curr->child;
		while (child != NULL)
		{
			enqueue(&q, child);
			child = child->brother;
		}
		//oslobodi memoriju svih bocica
		;
		free(curr->state.bottles);
		free(curr);
	}
}

void propagateWinPath(Node* winningNode) {
	Node* tmp = winningNode;
	while (tmp->father != NULL) {
		tmp->father->isWinPath = 1;
		tmp = tmp->father;
	}
}

Node* moveCurrentState(Node* currentState, int iz_bocice, int u_bocicu) //returns 0 if there are no winning moves
{
	Node* tmp = currentState->child;
	while (tmp != NULL) {
		if (tmp->iz_bocice == iz_bocice && tmp->u_bocicu == u_bocicu) {
			return tmp;
		}
		tmp = tmp->brother;
	}
	return NULL;
}

int winningPathExists(Node* currentState) {
	Node* tmp = currentState->child;
	if (currentState->win == GAME_WIN) return 1;
	while (tmp != NULL) {
		if (tmp->isWinPath) return 1;
		tmp = tmp->brother;
	}
	return 0;
}

Node* getHint(Node* currentState, int n, int* movesLeft) {
	Node* tmp = currentState->child;
	while (tmp != NULL) {
		if (tmp->isWinPath) {
			printf("Poured from %d to %d)\n", tmp->iz_bocice, tmp->u_bocicu);
			printGameState(tmp->state, n);
			(*movesLeft) -= 1;
			return tmp;
		}
		tmp = tmp->brother;
	}
	return NULL;
}

int findOneSolution(Node* currentState, int* steps, int* nrSteps) {
	Node* tmp = currentState->child;
	winState w = GAME_NOT_FINISHED;
	(*nrSteps) = 0;
	while (tmp != NULL) {
		if (tmp->isWinPath)
		{
			steps[*nrSteps] = tmp->iz_bocice;
			steps[*nrSteps + 1] = tmp->u_bocicu;
			(*nrSteps) += 2;
			w = tmp->win;
			tmp = tmp->child;
		}
		else {
			tmp = tmp->brother;
		}
	}
	if (w == GAME_WIN) return 1;
	return 0;
}

int initGameTree(Tree* gameTree, gameState game, int n, int k, int p) {
	//init the root
	gameTree->root = initializeNode();
	gameTree->root->depth = 0;
	gameTree->root->iz_bocice = -1;
	gameTree->root->u_bocicu = -1;
	gameTree->root->state = game;
	gameTree->root->isWinPath = 0;
	gameTree->root->father = NULL;
	gameTree->size = 1;
	gameTree->root->win = gameOver(gameTree->root->state, n, k, p);
	//printLevelNode(NULL,gameTree->root, gameTree->size, n);
	if (gameTree->root->win == GAME_WIN || gameTree->root->win == GAME_LOSE) return 1;
	bottle b1, b2;
	if (gameTree->root == NULL) return NULL;
	queue q;
	initQueue(&q);
	enqueue(&q, gameTree->root);
	while (!queueIsEmpty(&q)) {

		Node* curr = dequeue(&q);
		if (curr->win == GAME_WIN) continue;
		//Najverovatnije treba dodati gamestate lose i child i brother pokazivace na null
		for (int i = 0; i < n; i++)
		{
			//printf("#initGameTree - for 1\n"); //DEBUG
			if (curr->depth == p - 1) break;
			b1 = curr->state.bottles[i];
			//if (bottleHasSameColors(b1) || b1.length == 0) continue; //dont check solved or empty bottles (optimization)
			for (int j = i + 1; j < n; j++)
			{
				//printf("#initGameTree - for 2\n"); //DEBUG
				b2 = curr->state.bottles[j];
				//if (b1.length == BOTTLE_DEPTH && b2.length == 0) continue; //dont make unnecessary moves
				if (bottleHasSameColors(b2)) continue; //dont check solved bottles (optimization)
				if (canMoveBetweenBottles(&b1, &b2)) {
					gameState newState = deepCopy(curr->state, n, k);
					moveBetweenBottles(&newState.bottles[i], &newState.bottles[j]);
					winState win = gameOver(newState, n, k, p);
					Node* child = addChildNode(curr, i, j, newState, win);
					gameTree->size++;
					//printLevelNode(curr,child, gameTree->size, n);
					if (child->win == GAME_NOT_FINISHED) enqueue(&q, child);
					else if (child->win == GAME_WIN) {
						child->win = GAME_WIN;
						child->isWinPath = 1;
						enqueue(&q, child);
						propagateWinPath(child);
					}
				}
				if (canMoveBetweenBottles(&b2, &b1)) {
					gameState newState = deepCopy(curr->state, n, k);
					moveBetweenBottles(&newState.bottles[j], &newState.bottles[i]);
					winState win = gameOver(newState, n, k, p);
					Node* child = addChildNode(curr, j, i, newState, win);
					gameTree->size++;
					//printLevelNode(curr, child, gameTree->size, n);
					if (child->win == GAME_NOT_FINISHED) enqueue(&q, child);
					else if (child->win == GAME_WIN) {
						child->win = GAME_WIN;
						child->isWinPath = 1;
						enqueue(&q, child);
						propagateWinPath(child);
					}
				}
			}
		}
	}
	return 1;
}

int initGameState(gameState* game, int n, int k) {
	if (game == NULL) return -1;
	int maxSegments = (n - k) * BOTTLE_DEPTH; //ukupni broj razlicitih boja
	int currentSegments = 0; //trenutni broj izgenerisanih segmenata //treba popuniti n-k bocica, svake boje treba da ima tacno 4
	char color;
	int bottleSolved;
	char* nrColors;
	nrColors = (char*)calloc((n - k), sizeof(char));
	game->bottles = (bottle*)calloc(n, sizeof(bottle));
	if (nrColors == NULL || game->bottles == NULL) return -1;
	while (1) {
		bottleSolved = 0;
		for (int i = 0; i < (n - k); i++) {
			for (int j = 0; j < BOTTLE_DEPTH; j++) {
				color = getNextRand() % (n - k);
				while (nrColors[color] == BOTTLE_DEPTH)
				{
					color = getNextRand() % (n - k);
				}
				game->bottles[i].colors[j] = color;
				nrColors[color] += 1;

			}
			game->bottles[i].length = BOTTLE_DEPTH;
			if (bottleHasSameColors(game->bottles[i])) {
				bottleSolved = 1;
				memset(nrColors, 0, (n - k) * sizeof(char));
				memset(game->bottles, 0, n * sizeof(bottle));
				break;
			}
		}
		if (bottleSolved) continue; 
		else break; // if states are finished generating
	}
	game->win = GAME_NOT_FINISHED; 
	return 1;


}

int initTestGameState(gameState* game) {
	game->bottles = (bottle*)calloc(3, sizeof(bottle));
	game->bottles[0].colors[0] = 0;
	game->bottles[0].colors[1] = 1;
	game->bottles[0].colors[2] = 1;
	game->bottles[0].colors[3] = 0;
	game->bottles[0].length = 4;
	game->bottles[1].colors[0] = 1;
	game->bottles[1].colors[1] = 0;
	game->bottles[1].colors[2] = 0;
	game->bottles[1].colors[3] = 1;
	game->bottles[1].length = 4;
	game->bottles[2].length = 0;
	return 1;
}

void playAgain(int* selectedOption) {
	getchar();//pickup newline
	printf("\nDo you want to play again?\n");
	char c;
	do {
		printf("y/n: ");
		c = getchar();
	} while (c != 'n' && c != 'N' && c != 'y' && c != 'Y');
	if (c == 'n' || c == 'N')
	{
		*selectedOption = 0;
		return 0;
	}
	return 1;
}

void playGame() {
	int n = -1, k = -1, p = -1, selectedOption, gameInitialized = 0, movesLeft; 
	int nrSolvingSteps = -1;
	gameState game;
	Tree gameTree = { 0 };
	winState wState = GAME_NOT_FINISHED;
	Node* currentState = NULL;
	int* solutionSteps = NULL; // i-th is from bottle, i+1 th is to bottle
	while (!initLFGstate());//init PRNG
	selectedOption = getSelectedMenuOption();
	while (selectedOption) {
		switch (selectedOption) {
		case 1: {
			if (gameInitialized) {
				printf("Initial game state is already generated\n");
				break;
			}
			printf("Number of bottles: ");
			scanf("%d", &n);
			printf("Number of empty bottles: ");
			scanf("%d", &k);
			if (k >= n) {
				printf("[!] Error, number of empty bottles must not be larger than the number of full bottles\n");
				continue;
			}
			printf("Maximum number of steps: (recommended: 10): ");
			scanf("%d", &p);
			initGameState(&game, n, k);
			initGameTree(&gameTree, game, n, k, p);
			gameInitialized = 1;
			currentState = gameTree.root;
			solutionSteps = (int*)malloc(sizeof(int) * p * 2);
			if (!findOneSolution(currentState, solutionSteps, &nrSolvingSteps)) {
				printf("Generated game isn't solveable in the given number of steps\n");
				gameInitialized = 0;
				freeSubtree(gameTree.root);
				continue; //generate game again
			}
			movesLeft = p;
			printGameState(game, n);
			break;
		}
		case 2: {
			int  x, y;
			if (!gameInitialized) {
				printf("Game is not initialized!\n");
				break;
			}
			printGameState(currentState->state, n);
			while (1) {
				int canMove;
				printf("Move: (x,y): ");
				scanf("%d,%d", &x, &y);
				canMove = canMoveBetweenBottles(&(currentState->state).bottles[x], &(currentState->state).bottles[y]);
				if (canMove == 0) {
					printf("Invalid move!\n");
					continue;
				}
				else {
					//Dont generate subtrees
					if (bottleHasSameColors((currentState->state).bottles[x])) {
						printf("Bottle is already solved\n", x);
						break;
					}
					else if (bottleHasSameColors((currentState->state).bottles[y])) {
						printf("Bottle is already solved\n", y);
						break;
					}
					currentState = moveCurrentState(currentState, x, y);
					printGameState(currentState->state, n);
					movesLeft--;
					if (movesLeft == 0) {
						printf("You have run out of moves!\n");
						printColor(0); printf("\n================GAME OVER==============="); RESET;
						break;
					}
					if (currentState == NULL) {
						printf("Cant move to next state\n");
						break;
					}
					if (!winningPathExists(currentState)) {
						printf("It's impossible to win by playing from the current state! (Number of moves left: %d)\n", movesLeft);
						printColor(0); printf("\n================GAME OVER==============="); RESET;
						playAgain(&selectedOption);
						gameInitialized = 0;
						freeSubtree(gameTree.root);
						if (solutionSteps) free(solutionSteps);
						break;
					}
					else {
						wState = gameOver(currentState->state, n, k, movesLeft);
						if (wState == GAME_WIN) {
							printColor(1);
							printf("\n===================WIN==================");
							RESET;
							playAgain(&selectedOption);
							gameInitialized = 0;
							freeSubtree(gameTree.root);
							if (solutionSteps) free(solutionSteps);
							break;
						}
					}
					printf("Moves left: %d\n", movesLeft);
					break;
				}
			}
			break;
		}
		case 3: {
			if (!gameInitialized) { printf("Game is not initialized!\n"); break; }
			currentState = getHint(currentState, n, &movesLeft);
			wState = gameOver(currentState->state, n, k, movesLeft);
			if (wState == GAME_WIN) {
				printColor(1); printf("\n===================WIN=================="); RESET;
				playAgain(&selectedOption);
				gameInitialized = 0;
				freeSubtree(gameTree.root);
				if (solutionSteps) free(solutionSteps);
				break;
			}
			printf("Moves left: %d\n", movesLeft);
			break;
		}
		case 4: {
			if (!gameInitialized) { printf("Game is not initialized!\n"); break; }
			printf("Size of game tree % u\n", gameTree.size);
			printPreorder(gameTree.root, n);
			break;
		}
		case 5: {
			if (!gameInitialized) { printf("Game is not initialized!\n"); break; }
			solutionSteps = (int*)malloc(sizeof(int) * p * 2);
			findOneSolution(currentState, solutionSteps, &nrSolvingSteps);
			printSolutionSteps(solutionSteps, nrSolvingSteps);
			break;
		}
		case 6:
			if (!gameInitialized) { printf("Igra nije inicijalizovana!\n"); break; }
			freeSubtree(gameTree.root);
			gameInitialized = 0;
			break;
		case 7:
			n = 3;
			k = 1;
			p = 10;
			printf("Test state initialized\n");
			gameInitialized = 1;
			initTestGameState(&game);
			initGameTree(&gameTree, game, n, k, p);
			currentState = gameTree.root;
			movesLeft = p;
			break;
		default: printf("Error\n"); break;
		}
		if (selectedOption) selectedOption = getSelectedMenuOption();
		else printf("\n\n===============GAME OVER================\n");
	}
}

int main() {
	playGame();
	return 0;
}