#include "include/rtests.h"
#include <GL/glew.h>
#include <GL/gl.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// this turns into 1 if a test fails
static int retcode = 0;

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

// API misuse tests
void rTest_invalidEnum() {
	glEnable(0xffffffff);
	GLenum err = glGetError();
	assert(err == GL_INVALID_ENUM);
}
void rTest_invalidValue() {
	glLineWidth(-1.0f);
	GLenum err = glGetError();
	assert(err == GL_INVALID_VALUE);
}

// state machine robustness
void rTest_stateRecovr() {
	glEnable(0xdeadbeef);
	glGetError();
	glEnable(GL_BLEND);
	GLenum err = glGetError();
	assert(err == GL_NO_ERROR);
}

// resource management robustness
void rTest_outOfMemory() {
	GLuint tex;
	glGenTextures(1, &tex);
	for (int i = 0; i < 10000; i++) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8192, 8192, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		GLenum err = glGetError();
		if (err == GL_OUT_OF_MEMORY) {
			break;
		}
	}
}

// buffer and memory safety
void rTest_nullPtr() {
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	GLenum err = glGetError();
	EXPECT_GL_ERROR(err,
			(err != GL_NO_ERROR),
			"rTest_nullPtr failed.");
}
void rTest_oobDraw() {
	GLfloat data[6] = {0};
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, data);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLES, 0, 1000);
	GLenum err = glGetError();
	EXPECT_GL_ERROR(err,
			(err != GL_NO_ERROR),
			"rTest_oobDraw failed.");
//	assert(err != GL_NO_ERROR);
}

// shader robustness
void rTest_shaderCompilerError() {
	const char *bad = "void main() { gl_FragColor = vec4(1.0) }";
	GLuint s = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(s, 1, &bad, NULL);
	glCompileShader(s);
	GLint status;
	glGetShaderiv(s, GL_COMPILE_STATUS, &status);
	assert(status == GL_FALSE);
}
void rTest_invalidPrecision() {
	const char *bad = "precision superhighp float; void main(){}";
	GLuint s = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(s, 1, &bad, NULL);
	glCompileShader(s);
	GLint status;
	glGetShaderiv(s, GL_COMPILE_STATUS, &status);
	assert(status == GL_FALSE);
}

// draw pipeline robustness
void rTest_drawWOProgram() {
	glUseProgram(0);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	GLenum err = glGetError();

	EXPECT_GL_ERROR(err,
			(err != GL_NO_ERROR),
			"rTest_drawWOProgram failed.");
//	assert(err != GL_NO_ERROR);
}
void rTest_missingAttrib() {
	GLuint prog = glCreateProgram();
	glUseProgram(prog);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	GLenum err = glGetError();
	assert(err != GL_NO_ERROR);
}

// limit and capability tests
void rTest_maxTextureLimit() {
	GLint max;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, max + 1, max + 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	GLenum err = glGetError();
	assert(err == GL_INVALID_VALUE);
}

// error handling robustness
void rTest_errorFlood() {
	for (int i = 0; i < 10000; i++) {
		glEnable(0xffffffff);
	}

	GLenum err = glGetError();
	assert(err != GL_NO_ERROR);
}

// degenerate geometry/number handling
void rTest_NaNVertices() {
	GLfloat bad_data[6];
	bad_data[0] = 0.0f / 0.0f;  // NaN
	bad_data[1] = 1.0f / 0.0f;  // +Infinity
	bad_data[2] = -1.0f / 0.0f; // -Infinity
	bad_data[3] = 1.0f;
	bad_data[4] = 1.0f;
	bad_data[5] = 1.0f;

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, bad_data);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, 3);
	GLenum err = glGetError();
	assert(err == GL_NO_ERROR);
}

