#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

// 공유 메모리 크기와 파일 읽기 버퍼 크기를 정의하는 매크로
#define SHARED_MEM_SIZE 4096
#define BUFFER_SIZE 1024

// 시그널 핸들러 함수
// SIGINT 신호(Ctrl+C)를 받으면 현재 수신한 시그널 번호를 출력
void signal_handler(int signum) {
    printf("수신한 시그널: %d\n", signum);
}

int main() {
    int *shared_mem; // 공유 메모리를 가리키는 포인터
    int fd[2]; // 파이프 파일 디스크립터 배열 (읽기용, 쓰기용)
    pid_t pid; // 프로세스 ID를 저장할 변수

    char *filename = "example.txt"; // 파일 이름
    char buffer[BUFFER_SIZE]; // 파일 읽기 버퍼
    int file_fd; // 파일 디스크립터
    struct stat file_stat; // 파일 상태 정보를 저장할 구조체

    // SIGINT 시그널에 대한 핸들러 등록
    // Ctrl+C를 입력하면 signal_handler 함수 호출
    signal(SIGINT, signal_handler);

    // 익명 공유 메모리 생성 및 초기화
    // mmap 시스템 호출을 사용하여 공유 메모리를 할당하고 초기값 100을 설정
    shared_mem = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    *shared_mem = 100; // 공유 메모리 초기값 설정

    // 파이프 생성
    // pipe 시스템 호출을 사용하여 파이프를 생성
    // 파이프는 프로세스 간 통신을 위해 사용
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }

    // 부모 프로세스 ID 출력
    printf("부모 프로세스 ID: %d\n", getpid());

    // 자식 프로세스 생성
    // fork 시스템 호출을 사용하여 자식 프로세스를 생성
    pid = fork();

    if (pid < 0) {
        // fork 실패
        fprintf(stderr, "Fork 실패\n");
        return 1;
    } else if (pid == 0) {
        // 자식 프로세스 코드
        close(fd[0]); // 읽기 끝 닫기 (자식 프로세스에서는 읽기 끝을 사용하지 않음)
        printf("자식 프로세스: 공유 메모리 값: %d\n", *shared_mem); // 공유 메모리 값 출력
        write(fd[1], "Hello from child", 17); // 파이프에 데이터 쓰기
        printf("자식 프로세스: 파이프에 데이터 쓰기 완료\n");
        printf("자식 프로세스 ID: %d\n", getpid()); // 자식 프로세스 ID 출력
        printf("부모 프로세스 ID (자식에서): %d\n", getppid()); // 부모 프로세스 ID 출력
    } else {
        // 부모 프로세스 코드
        close(fd[1]); // 쓰기 끝 닫기 (부모 프로세스에서는 쓰기 끝을 사용하지 않음)
        wait(NULL); // 자식 프로세스 종료 대기
        printf("자식 프로세스 종료 완료\n");
        printf("부모 프로세스: 공유 메모리 값: %d\n", *shared_mem); // 공유 메모리 값 출력
        read(fd[0], buffer, sizeof(buffer)); // 파이프에서 데이터 읽기
        printf("부모 프로세스: 파이프에서 읽은 데이터: %s\n", buffer); // 읽은 데이터 출력

        // 파일 생성 및 쓰기
        // example.txt 파일을 생성하고 내용 쓰기
        file_fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (file_fd == -1) {
            perror("open");
            return 1;
        }

        snprintf(buffer, sizeof(buffer), "안녕하세요 이건 시스템 프로그래밍 과제 파일입니다.\n");
        write(file_fd, buffer, strlen(buffer)); // 파일에 데이터 쓰기
        close(file_fd);

        printf("'%s' 파일 입력이 정상적으로 완료되었습니다.\n", filename);

        // 파일 정보 가져오기
        // stat 시스템 호출을 사용하여 파일의 상태 정보를 가져오기
        if (stat(filename, &file_stat) == -1) {
            perror("stat");
            return 1;
        }

        printf("\n파일 정보:\n");
        printf("  파일 크기: %ld bytes\n", file_stat.st_size); // 파일 크기 출력
        printf("  최근 수정 시간: %s", ctime(&file_stat.st_mtime)); // 파일 수정 시간 출력

        // 파일 내용 읽기
        // open 시스템 호출을 사용하여 example.txt 파일을 읽기 모드로 열기
        file_fd = open(filename, O_RDONLY);
        if (file_fd == -1) {
            perror("open");
            return 1;
        }

        memset(buffer, 0, sizeof(buffer)); // 버퍼 초기화
        ssize_t bytes_read = read(file_fd, buffer, sizeof(buffer) - 1); // 파일 읽기
        close(file_fd);

        if (bytes_read == -1) {
            perror("read");
            return 1;
        }

        // 파일 내용 출력
        printf("\n파일 내용:\n%s\n", buffer);

        // 시스템 정보 출력
        printf("\n시스템 정보:\n");
        printf("프로세스 ID: %d\n", getpid());
        printf("부모 프로세스 ID: %d\n", getppid());
        printf("사용자 ID: %d\n", getuid());
        printf("그룹 ID: %d\n", getgid());

        // ls 명령어 실행
        printf("\n파일에 대해 ls 명령어 실행 중:\n");
        pid = fork(); // 자식 프로세스 생성
        if (pid == 0) {
            execl("/bin/ls", "ls", "-l", filename, NULL); // ls 명령어 실행
            perror("exec");
            exit(1);
        } else if (pid < 0) {
            perror("fork");
            return 1;
        } else {
            wait(NULL); // 자식 프로세스 종료 대기
            printf("\nls 명령어 실행 완료.\n");
        }
    }

    return 0;
}
