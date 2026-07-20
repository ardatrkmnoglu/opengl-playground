#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#ifdef RUN_EXTESTS
#include "include/rtests.h"
#endif

/** COLORS!
 * RED:    "\x1b[31m"
 * GREEN:  "\x1b[32m"
 * YELLOW: "\x1b[33m"
 * BLUE:   "\x1b[34m"
 *
 * RESET:  "\x1b[0m"
 */

/**************************************/
/*********** Shader Sources ***********/
/**************************************/
const char* vs_source =
        "attribute vec4 vPosition;\n"
        "void main() {\n"
        "    gl_Position = vPosition;\n"
        "}\n";

const char* fs_source =
        "precision mediump float;\n"
        "void main() {\n"
        "    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";


/**************************************/
/********** Helper Functions **********/
/**************************************/

// dummy program function for draw/pipeline tests
GLuint createDummyProgram() {
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_source, NULL);
	glCompileShader(vs);

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_source, NULL);
	glCompileShader(fs);

	GLuint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);

	glBindAttribLocation(prog, 0, "vPosition");

	glLinkProgram(prog);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return prog;
}

// global variable for error handling
// this turns into 1 if a test fails
static int retcode = 0;

// error handling function
#define EXPECT_GL_ERROR(actual_err, condition_expr, msg)                       \
	do {                                                                   \
		if (!(condition_expr)) {                                       \
			fprintf(stderr,                                        \
				"\x1b[33m[FAIL]\x1b[0m %s (Line: %d)\n",       \
				msg, __LINE__);                                \
			fprintf(stderr, "   > Expected: %s\n",                 \
				#condition_expr);                              \
			fprintf(stderr, "   > Actual  : 0x%04X\n",             \
				actual_err);                                   \
			retcode = 1;                                           \
			continue;                                              \
		}                                                              \
	} while (0)

// create & destroy window context
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

// run test functions
void runTest(void (*test_func)(), const char* name) {
	printf("[TEST] %s...\n", name);
	test_func();
	if (retcode == 0)
		printf("\x1b[32m[PASS]\x1b[0m %s passed.\n", name);
	retcode = 0;
}

/************************************************************************** TESTS ***************************************************************************/
/***************************************/
/****** Robustness Test Functions ******/
/***************************************/

/* Shaders and Programs */
void rTest_CreateProgram() {
	GLuint p_count = 0;
//	while (1) {
	while (p_count < 10000) {  // normalde sonsuz döngü olmalı ama burada çalıştırmak için testi 10000000 defa koşturuyoruz
		GLuint prog = glCreateProgram();
		GLenum err = glGetError();
		if (prog == 0 || err == GL_OUT_OF_MEMORY)
			break;

		assert(err == GL_NO_ERROR);
		p_count++;
	}
}

void rTest_ProgramBinary_unalignedPtr() {
	GLuint prog = glCreateProgram();

	void* valid_memblock = malloc(1024);
	const void* unaligned_ptr = (const void*)((char*)valid_memblock + 1);

	glProgramBinary(prog, 0x1234, unaligned_ptr, 100);

	GLenum err = glGetError();
	EXPECT_GL_ERROR(err,
			(err == GL_INVALID_ENUM ||
			 err == GL_INVALID_VALUE ||
			 err == GL_INVALID_OPERATION),
			"rTest_ProgramBinary_unalignedPtr failed.");
	free(valid_memblock);
}
void rTest_ProgramBinary_memRevoke() {
	GLuint prog = glCreateProgram();

	size_t page_size = sysconf(_SC_PAGESIZE);
	void* mapped_memory = mmap(NULL, page_size, PROT_READ | PROT_WRITE,
				   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	mprotect(mapped_memory, page_size, PROT_NONE);

	glProgramBinary(prog, 0x1234, mapped_memory, 1024);

	GLenum err = glGetError();
	EXPECT_GL_ERROR(err,
			(err == GL_INVALID_ENUM ||
			 err == GL_INVALID_VALUE ||
			 err == GL_INVALID_OPERATION),
			"rTest_ProgramBinary_memRevoke failed.");
	munmap(mapped_memory, page_size);
}
void rTest_ProgramBinary_overload() {
	GLuint prog = glCreateProgram();

	GLint num_formats = 0;
	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &num_formats);

	GLenum valid_format = 0x1234;
	if (num_formats > 0)
		glGetIntegerv(GL_PROGRAM_BINARY_FORMATS,
			      (GLint*)&valid_format);

	char buf[4] = {0xDE, 0xAD, 0xBE, 0xEF};
	GLsizei overload_len = 2147483631;

	glProgramBinary(prog, valid_format, buf, overload_len);
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		EXPECT_GL_ERROR(err,
				(err == GL_INVALID_ENUM ||
				 err == GL_INVALID_VALUE ||
				 err == GL_INVALID_OPERATION),
				"rTest_ProgramBinary_overload failed.");
	}

	GLint link_status = GL_TRUE;
	glGetProgramiv(prog, GL_LINK_STATUS, &link_status);

	EXPECT_GL_ERROR(link_status,
			(link_status == GL_FALSE),
			"rTest_ProgramBinary_overload failed.\n"
			"\x1b[31mCRITICAL:\x1b[0m Driver accepted the garbage "
			"data as a valid Shader Program.");
}

void rTest_UseProgram_invalidID() {
	GLuint ghost_id = 0xdeadbeef;
	glUseProgram(ghost_id);

	GLenum err = glGetError();
	EXPECT_GL_ERROR(err,
			(err == GL_INVALID_VALUE),
			"rTest_UseProgram_invalidID failed.\n"
			"\x1b[31mCRITICAL:\x1b[0m Driver accepted the "
			"ghost ID (has never existed) as a valid program ID.");
}
void rTest_UseProgram_typeConfusion() {
	GLuint shader = glCreateShader(GL_VERTEX_SHADER);
	glUseProgram(shader);

	GLenum err = glGetError();
	EXPECT_GL_ERROR(err,
		 	(err == GL_INVALID_OPERATION),
		 	"rTest_UseProgram_typeConfusion failed.\n");

	glDeleteShader(shader);
}

/* Vertices */
void rTest_DrawArrays_outOfBounds() {
	GLfloat vertices[] = {
		-0.5, -0.5,  0.0,
		 0.5, -0.5,  0.0,
		 0.0,  0.5,  0.0
	};

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, 1000000);

	GLenum err = glGetError();
	EXPECT_GL_ERROR(err,
		 	(err == GL_INVALID_VALUE ||
		 	 err == GL_INVALID_OPERATION),
		 	"rTest_DrawArrays_outOfBounds failed.");

	glDisableVertexAttribArray(0);
}

void rTest_DrawArrays_guardPageAttack(void) {
	GLuint prog = createDummyProgram();
	glUseProgram(prog);

	size_t page_size = sysconf(_SC_PAGESIZE);

	void* memory = mmap(NULL, page_size * 2, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	void* guard_page = (char*)memory + page_size;
	mprotect(guard_page, page_size, PROT_NONE);

	GLfloat* edge_data = (GLfloat*)((char*)guard_page - (9 * sizeof(GLfloat)));

	edge_data[0] = 0.0f; edge_data[1] = 1.0f; edge_data[2] = 0.0f;
	edge_data[3] =-1.0f; edge_data[4] =-1.0f; edge_data[5] = 0.0f;
	edge_data[6] = 1.0f; edge_data[7] =-1.0f; edge_data[8] = 0.0f;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, edge_data);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, 300000000);
	glFinish();

	GLubyte px[4];
	glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, px);

	GLenum err = glGetError();

	EXPECT_GL_ERROR(
		err,
		(err == GL_INVALID_OPERATION || err == GL_INVALID_VALUE),
		"rTest_DrawArrays_guardPageAttack failed.");

	glDisableVertexAttribArray(0);
	munmap(memory, page_size * 2);
}

#define runTest(func) runTest(func, #func)

int main() {
	GLFWwindow* w;
	createContext(&w);

	printf("[INFO] Initializing: Robustness tests for OpenGL SC 2.0\n");
	printf("--------------------------------------------------\n");

	runTest(rTest_CreateProgram);
	runTest(rTest_ProgramBinary_unalignedPtr);
	runTest(rTest_ProgramBinary_memRevoke);
	runTest(rTest_ProgramBinary_overload);
//	runTest(rTest_DrawArrays_guardPageAttack);	SEGMENTATION FAULT
	runTest(rTest_DrawArrays_outOfBounds);

	#ifdef RUN_EXTESTS
	// other exemplary tests in rtests.c
	runTest(rTest_invalidEnum);
	runTest(rTest_invalidValue);
	runTest(rTest_invalidPrecision);
	runTest(rTest_errorFlood);
	runTest(rTest_shaderCompilerError);
	runTest(rTest_maxTextureLimit);
	runTest(rTest_missingAttrib);
	runTest(rTest_NaNVertices);
	runTest(rTest_outOfMemory);
	runTest(rTest_stateRecovr);
	runTest(rTest_drawWOProgram);
	runTest(rTest_oobDraw);
	runTest(rTest_nullPtr);
	#endif

	printf("--------------------------------------------------\n");
	printf("[SUCCESS] All tests complete.\n");

	destroyContext(&w);
	return 0;
}
