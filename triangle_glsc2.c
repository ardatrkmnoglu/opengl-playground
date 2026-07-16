#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

// vertex shader kaynak kodu
const char* vertexShaderSource = "#version 100\n" // OpenGL SC 2.0
	"attribute vec3 aPos;\n"
	"void main()\n"
	"{\n"
	"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
	"}\0";

// fragment shader kaynak kodu
const char* fragmentShaderSource = "#version 100\n"
	"precision mediump float;\n"
	"void main()\n"
	"{\n"
	"   gl_FragColor = vec4(1.0f, 0.3f, 0.3f, 1.0f);\n" // vec4(R, G, B, A)
	"}\0";

int main() {
	// ***** #1 *****
	// GLFW başlatma
	if (!glfwInit()) {
		fprintf(stderr, "Could not initialize GLFW.\n");
		return -1;
	}

	// OpenGL ES/SC 2.0
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	GLFWwindow* window = glfwCreateWindow(1200, 900, "Deneme", NULL, NULL);
	if (!window) {
		fprintf(stderr, "Could not create window.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// ***** #2 *****
	// GLEW başlatma, driverların uyanması ve fonksiyonların yüklenmesi
	glewInit();
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Could not initialize GLEW.\n");
		return -1;
	}

	// ***** #3 *****
	// shader compile etme (GLSL vertex ve fragment shader kısmı)
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// ***** #4 *****
	// veriyi GPU belleğine yazma
	float vertices[] = {
		-0.5, -0.5,  0.0,
		 0.5, -0.5,  0.0,
		 0.0,  0.5,  0.0
	};

	// VAO ve VBO initialize et, bağla ve verileri GPU belleğine kopyala
	GLuint VBO;
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	// bağlantıyı koparma işlemi (unbind)
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// ***** #5 *****
	// render loop (ana döngü)
	while (!glfwWindowShouldClose(window)) {
		// ekranın temizlenmesi ve programın çalıştırılması
		glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shaderProgram);

		// üçgen çizimi
		glDrawArrays(GL_TRIANGLES, 0, 3);

		// double buffering
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// ***** #6 *****
	// cleanup
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);

	glfwTerminate();

	return 0;
}
