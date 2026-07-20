#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

/* Create & Destroy Window Context */
int createContext(GLFWwindow** window) {
	if (!glfwInit())
		return -1;
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	*window = glfwCreateWindow(640, 480, "test", NULL, NULL);
	if (!*window) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(*window);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
		return -1;
	glGetError();
	return 0;
}
void destroyContext(GLFWwindow** window) {
	glfwDestroyWindow(*window);
	glfwTerminate();
}

/* Isolate & Run Test Functions */
void runTest(const char* test_name, void (*test_func)(void)) {
	printf("[TEST] %s... \n", test_name);
	fflush(stdout);

	pid_t pid = fork();

	if (pid < 0) {
		printf("\033[0;31m[FORK FAILED]\033[0m\n");
		return;
	}

	if (pid == 0) {
		// child process
		GLFWwindow* window;
		if (createContext(&window)) {
			exit(99);
		}

		test_func();

		glfwDestroyWindow(window);
		glfwTerminate();
		exit(0);
	} else {
		// parent process
		int status;
		waitpid(pid, &status, 0);

		if (WIFEXITED(status)) {
			int exit_code = WEXITSTATUS(status);
			if (exit_code == 0) {
				printf("\033[0;32mPASS\033[0m\n");
			} else if (exit_code == 99) {
				printf("\033[0;33mCONTEXT ERROR\033[0m\n");
			} else {
				printf(
				    "\033[0;31mFAIL (Exit Code: %d)\033[0m\n",
				    exit_code);
			}
		} else if (WIFSIGNALED(status)) {
			int sig = WTERMSIG(status);
			if (sig == SIGSEGV) {
				printf("\033[0;31mCRASHED (Segmentation "
				       "Fault)\033[0m\n");
			} else if (sig == SIGABRT) {
				printf("\033[0;31mABORTED\033[0m\n");
			} else if (sig == SIGKILL) {
				printf("\033[0;31mKILLED\033[0m\n");
			} else {
				printf("\033[0;31mTERMINATED (Signal: "
				       "%d)\033[0m\n",
				       sig);
			}
		}
	}
}

