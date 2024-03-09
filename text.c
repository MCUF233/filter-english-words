#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> // 用于布尔值
#include <ctype.h>
#include <windows.h>


#define MAX_WORDS 6000 // 最大单词数量
#define MAX_TXT 500 // 最大文本数量

// 结构体定义用于存储单词和频率
typedef struct {
    char word[50]; // 假设单词长度不超过50个字符
    int frequency;
} Word;

// 全局变量声明
Word* words; // 改为指针
int wordCount = 0; // 存储的单词数量

// 快速排序的辅助函数 - 交换两个 Word 结构
void swap(Word* a, Word* b) {
    Word t = *a;
    *a = *b;
    *b = t;
}
// 按频率分区函数
int partition(int low, int high) {
    int pivot = words[high].frequency;
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        if (words[j].frequency > pivot) { // 注意这里的比较是“大于”
            i++;
            swap(&words[i], &words[j]);
        }
    }
    swap(&words[i + 1], &words[high]);
    return (i + 1);
}
// 快速排序的主函数
void quickSort(int low, int high) {
    if (low < high) {
        int pi = partition(low, high);

        quickSort(low, pi - 1);
        quickSort(pi + 1, high);
    }
}

// 计算字符串中中文字符的数量
int chineseCharCount(const char* str) {
    int count = 0;
    while (*str) {
        if ((unsigned char)*str >= 0x80) {
            count++;  // 假设使用的是UTF-8编码
            str += 3; // 跳过UTF-8编码的中文字符
        }
        else {
            str++;
        }
    }
    return count;
}

//实现去除频率大于等于3,且字母数小于等于4的单词
//void removeSpecificWords() {
//    int j = 0;
//    for (int i = 0; i < wordCount; i++) {
//        if (!(words[i].frequency >= 3 && strlen(words[i].word) <= 4)) {
//            words[j++] = words[i];
//        }
//    }
//    wordCount = j; // 更新数组中单词的数量
//}

// 函数声明
void readFile(char* filename);
void processText(char* text);
void countFrequency(char* word);
void sortWords();
void filterWords(int minFrequency, int minLength, bool filterEnabled);//筛选>=Frequency >=Length
void printWords();

// 主函数
int main() {
    // 设置控制台代码页为UTF-8
    SetConsoleOutputCP(CP_UTF8);

    int num = 0;
    // 动态分配内存
    words = (Word*)malloc(MAX_WORDS * sizeof(Word));
    if (words == NULL) {
        fprintf(stderr, "内存分配失败\n");
        return 1;
    }

    char filename[] = "E:\\VS Projects\\extract english words\\file data\\input.txt";
    readFile(filename);

    //打印当前wordCount
    printf("%d",wordCount);
    // 在读取文本后检查是否有越界
    if (wordCount >= MAX_WORDS) {
        fprintf(stderr, "警告：达到单词数量上限，可能有部分单词未处理。\n");
    }

    // 调用其他函数处理文本、统计频率等
    sortWords(); // 排序单词
    //printWords();
    //printf("%d", wordCount);
    //尝试重定向 stdout 到文件
    FILE* file;
    errno_t err = freopen_s(&file, "E:\\VS Projects\\extract english words\\file data\\output.txt", "w", stdout);
    if (err != 0) {
        fprintf(stderr, "无法打开文件用于写入\n");
        free(words);
        return 1;
    }

    printf("\nFiltered Words:\n");
    filterWords(10, 5, true);//去除>=Frequency&&<=Length || length<=3的单词
    printWords();

    // 关闭文件并恢复标准输出
    if (file != NULL) {
        fclose(file);
    }
    err = freopen_s(&file, "CON", "w", stdout);
    if (err != 0) {
        fprintf(stderr, "无法恢复标准输出\n");
        free(words);
        return 1;
    }

    free(words);
    getch();
    return 0;
}

// 读取文件内容
void readFile(char* filename) {
    FILE* file;
    bool isFirstLine = true; // 首行标志

    char buffer[MAX_TXT]; // 用于存储文件中的文本
    // 初始化 buffer 数组
    memset(buffer, 0, sizeof(buffer));

    // 使用 fopen_s 打开文件
    errno_t err = fopen_s(&file, filename, "r");
    if (err != 0) {
        fprintf(stderr, "无法打开文件 %s\n", filename);
        exit(1);
    }

    // 读取文件内容
    while (fgets(buffer, sizeof(buffer), file) != NULL) {

        if (strlen(buffer) == sizeof(buffer) - 1 && buffer[sizeof(buffer) - 2] != '\n') {
            fprintf(stderr, "警告：一行文本可能太长，无法完全读取。\n");
        }
        printf("%s", buffer); // 打印每一行
        // 这里可以加入更多的处理，比如分割单词等
        processText(buffer); // 处理每行文本
    }
    for (int n = 0; n < 2; n++) {
        printf("\n");
    }
    // 关闭文件
    fclose(file);
}


// 处理文本，分割成单词
void processText(char* text) {
    const char* delimiters = " ,;.!?\r\n\"";
    char* token;
    char* next_token = NULL;
    char processedWord[50];
    int len;

    // 使用 strtok_s 获取第一个单词
    token = strtok_s(text, delimiters, &next_token);

    while (token != NULL) {
        len = 0;
        for (int i = 0; token[i] != '\0';) {
            if (isalpha((unsigned char)token[i])) {
                if (len < sizeof(processedWord) - 1) {
                    processedWord[len++] = tolower((unsigned char)token[i]);
                }
                else {
                    fprintf(stderr, "警告：单词长度超过预设限制。\n");
                    break;
                }
                i++;
            }
            else {
                break;
            }
        }

        processedWord[len] = '\0'; // 确保字符串以空字符结尾

        if (len > 0) { // 如果有有效的单词，统计它的频率
            countFrequency(processedWord);
        }

        // 继续获取下一个单词
        token = strtok_s(NULL, delimiters, &next_token);
    }
}

// 统计单词频率
void countFrequency(char* word) {
    // 实现频率统计
    // 遍历数组查找单词
    for (int i = 0; i < wordCount; i++) {
        if (strcmp(words[i].word, word) == 0) {
            // 如果找到了单词，增加频率
            words[i].frequency++;
            return;
        }
    }

    // 如果单词不在数组中，添加它
    if (wordCount < MAX_WORDS) {
        strcpy_s(words[wordCount].word, sizeof(words[wordCount].word), word);
        words[wordCount].frequency = 1;
        wordCount++;
    }
    else {
        // 如果达到数组容量上限，打印错误或扩展数组
        fprintf(stderr, "已达到单词存储上限\n");
    }
}

// 对单词进行排序(快速排序)
void sortWords() {
    quickSort(0, wordCount - 1);
    //removeSpecificWords(); // 过滤单词
}

// 筛选单词
void filterWords(int minFrequency, int minLength, bool filterEnabled) {
    if (!filterEnabled) {
        return; // 如果筛选不启用，则直接返回
    }

    int j = 0;
    for (int i = 0; i < wordCount; i++) {
        if ((words[i].frequency < minFrequency || strlen(words[i].word) > minLength) && strlen(words[i].word) >= 4) {
            words[j++] = words[i];
        }
    }
    wordCount = j; // 更新数组中单词的数量
}

// 打印单词和频率
void printWords() {
    // 实现打印功能
   /* printf("单词\t频率\n");
    printf("---------------\n");
    for (int i = 0; i < wordCount; i++) {
        printf("%s\t%d\n", words[i].word, words[i].frequency);*/
    const int wordWidth = 20; // 单词列的宽度
    const int freqWidth = 10; // 频率列的宽度

    int adjustedWidth;

    // 计算调整后的宽度
    adjustedWidth = wordWidth - chineseCharCount("单词") * 2;
    printf("%-*s%*s\n", adjustedWidth, "单词", freqWidth, "频率");

    adjustedWidth = wordWidth - chineseCharCount("--------------------") * 2;
    printf("%-*s%*s\n", adjustedWidth, "--------------------", freqWidth, "----------");

    for (int i = 0; i < wordCount; i++) {
        adjustedWidth = wordWidth - chineseCharCount(words[i].word) * 2;
        printf("%-*s%*d\n", adjustedWidth, words[i].word, freqWidth, words[i].frequency);
    }
}