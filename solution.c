#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>

#define PERMS 0664
#define CHARS 26

typedef struct
{
    long mtype;
    int message;
} queue_message;

typedef struct trieNode
{
    struct trieNode *children[CHARS];
    bool isValidWord;
    int count;
} trieNode;

trieNode *trieNodeCreate()
{
    trieNode *newNode = malloc(sizeof(trieNode));
    for (int i = 0; i < CHARS; i++)
    {
        newNode->children[i] = NULL;
    }
    newNode->isValidWord = false;
    newNode->count = 0;
    return newNode;
}

bool trieNodeInsert(trieNode **root, char *word)
{
    if (*root == NULL)
    {
        *root = trieNodeCreate();
    }
    trieNode *node = *root;
    int length = strlen(word);
    for (int i = 0; i < length; i++)
    {
        int index = word[i] - 'a';
        if (node->children[index] == NULL)
        {
            node->children[index] = trieNodeCreate();
        }
        node = node->children[index];
    }
    if (node->isValidWord)
    {
        node->count++;
        return false;
    }
    else
    {
        node->isValidWord = true;
        node->count = 1;
        return true;
    }
}

void fileWordsToTrie(const char *fileName, trieNode **root, int maxWordLength)
{
    FILE *file = fopen(fileName, "r");
    if (file == NULL)
    {
        perror("Error: Could not open words file");
        return;
    }
    char *word = malloc(maxWordLength * sizeof(char));
    if (word == NULL)
    {
        perror("Error: Could not allocate memory");
        fclose(file);
        return;
    }
    while (fscanf(file, "%s", word) == 1)
    {
        trieNodeInsert(root, word);
    }
    free(word);
    fclose(file);
}

int trieNodeCountAfterRotation(trieNode *root, const char *word, int cesear_key)
{
    if (root == NULL)
    {
        return 0;
    }
    trieNode *node = root;
    int length = strlen(word);
    for (int i = 0; i < length; i++)
    {
        int indexAfterRotation = (word[i] - 'a' + cesear_key) % 26;
        if (node->children[indexAfterRotation] == NULL)
        {
            return 0;
        }
        node = node->children[indexAfterRotation];
    }
    if (node->isValidWord)
        return node->count;
    else
        return 0;
}

void freeTrie(trieNode *root)
{
    if (root == NULL)
    {
        return;
    }
    for (int i = 0; i < CHARS; i++)
    {
        if (root->children[i] != NULL)
        {
            freeTrie(root->children[i]);
        }
    }
    free(root);
}

int main(int argc, char *argv[])
{
    queue_message msg;
    if (argc != 2)
    {
        printf("Wrong Format. Have to pass an argument\n");
        return -1;
    }
    int fileNumber = atoi(argv[1]);
    if (fileNumber < 1 || fileNumber > 99)
    {
        printf("Error: File number should be between 1 and 99.\n");
        return -2;
    }
    char inputFileName[15];
    sprintf(inputFileName, "input%d.txt", fileNumber);
    char wordsFileName[15];
    sprintf(wordsFileName, "words%d.txt", fileNumber);
    FILE *inputFile = fopen(inputFileName, "r");
    if (inputFile == NULL)
    {
        perror("Error: Could not open input file");
        return -3;
    }
    int N, maxWordLength, shmKey, msgqKey;
    fscanf(inputFile, "%d", &N);
    fscanf(inputFile, "%d", &maxWordLength);
    fscanf(inputFile, "%d", &shmKey);
    fscanf(inputFile, "%d", &msgqKey);
    fclose(inputFile);
    int shmId;
    char(*shmptr)[N][maxWordLength];
    if ((shmId = shmget(shmKey, sizeof(char[N][N][maxWordLength]), PERMS)) == -1)
    {
        perror("Error: Could not access shared memory");
        return -4;
    }
    if ((shmptr = shmat(shmId, NULL, 0)) == (void *)-1)
    {
        perror("Error: Could not attach shared memory");
        return -5;
    }
    int msgqId;
    if ((msgqId = msgget(msgqKey, PERMS)) == -1)
    {
        perror("Error: Could not access message queue");
        return -6;
    }
    trieNode *root = NULL;
    fileWordsToTrie(wordsFileName, &root, maxWordLength);
    char firstDiagonal[maxWordLength];
    strncpy(firstDiagonal, shmptr[0][0], maxWordLength - 1);
    firstDiagonal[maxWordLength - 1] = '\0';
    int firstDiagonalCount = trieNodeCountAfterRotation(root, firstDiagonal, 0);
    msg.mtype = 1;
    msg.message = firstDiagonalCount;
    if (msgsnd(msgqId, &msg, sizeof(msg) - sizeof(msg.mtype), 0) == -1)
    {
        perror("Error: Could not send first word count to the queue");
        return -7;
    }
    if (msgrcv(msgqId, &msg, sizeof(msg) - sizeof(msg.mtype), 2, 0) == -1)
    {
        perror("Error: Could not receive message from the queue");
        return -8;
    }
    int cesearKey = msg.message;
    for (int k = 1; k < (2 * N) - 1; k++)
    {
        int totalDiagonalSum = 0;
        for (int i = 0; i < N; i++)
        {
            int j = k - i;
            if (j >= 0 && j < N)
            {
                totalDiagonalSum += trieNodeCountAfterRotation(root, shmptr[i][j], cesearKey);
            }
        }
        msg.mtype = 1;
        msg.message = totalDiagonalSum;
        if (msgsnd(msgqId, &msg, sizeof(msg) - sizeof(msg.mtype), 0) == -1)
        {
            perror("Error: Could not send message to the queue");
            return -9;
        }
        if (k < (2 * N) - 2)
        {
            if (msgrcv(msgqId, &msg, sizeof(msg) - sizeof(msg.mtype), 2, 0) == -1)
            {
                perror("Error: Could not receive message from the queue");
                return -10;
            }
            cesearKey = msg.message;
        }
    }
    if (shmdt(shmptr) == -1)
    {
        perror("Error: Could not detach shared memory");
        return -11;
    }
    freeTrie(root);
    return 0;
}
