#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <camera.h>
#include <vector>
#include <glm/gtx/rotate_vector.hpp>µ
#include <cmath>
#include <glm/gtx/quaternion.hpp>
#include <shader.h>
#include <stb_image.h>
#include <model.h>

void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void processInput(GLFWwindow* window);
void processMouseInput(GLFWwindow* window, double x, double y);
void drawRail(const std::vector<glm::vec3>& path, const Shader& shader, unsigned int VAO);
void drawCurvedTies(const std::vector<glm::vec3>& tiePositions, const Shader& shader, unsigned int VAO);
void drawStraightTies(const std::vector<glm::vec3>& tiePositions, const Shader& shader, unsigned int VAO);
void drawBezierTies(const std::vector<glm::vec3>& tiePositions, const Shader& shader, unsigned int VAO);
int generateCylinder(int segments, unsigned int& vertexCount);
void generateTiesBetweenCurvedRails(const std::vector<glm::vec3>& leftRailPath, const std::vector<glm::vec3>& rightRailPath, std::vector<glm::vec3>& ties);
void generateTiesBetweenStraightRails(const std::vector<std::pair<glm::vec3, glm::vec3>>& leftSegments, const std::vector<std::pair<glm::vec3, glm::vec3>>& rightSegments, std::vector<glm::vec3>& ties, int numTies);
void generateTiesBetweenBezierRails(const std::vector<glm::vec3>& leftRailPath, const std::vector<glm::vec3>& rightRailPath, std::vector<glm::vec3>& tiePositions);
std::vector<glm::vec3> generateCurvedRailAroundCenter(glm::vec3 center, float radius, float startAngle, float angle, int numSegments, float offsetDistance);
glm::vec3 calculateMidpoint();
std::vector<glm::vec3> calculateCenterRail(const std::vector<glm::vec3>& leftRail, const std::vector<glm::vec3>& rightRail);
int findClosestXZIndex(const glm::vec3& centerPoint, const std::vector<glm::vec3>& centerRail);
std::vector<glm::vec3> generateBezierHill(glm::vec3 start, glm::vec3 direction, float segmentLength, int numSegments, float hillHeight);
void drawMidRail(const std::vector<glm::vec3>& path, float y, const Shader& shader, unsigned int VAO);

// Window
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Camera
Camera camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
bool firstMouse = true;
float lastX = 800 / 2.0f;
float lastY = 600 / 2.0f;
bool firstPersonMode = false;

// Wheels
float wheelRadius = 0.1f;
float railTopY = -0.15f;
float wheelCenterY = railTopY + wheelRadius + 0.15f;

glm::vec3 wheelPositions[4] = {
    { -0.45f, wheelCenterY, -0.2f},
    { 0.45f, wheelCenterY, -0.2f},
    { -0.45f, wheelCenterY,  0.2f},
    { 0.45f, wheelCenterY,  0.2f}
};

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};

// Body
float cubeVertices[] = {
    // positions          // tex coords
    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,   0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,   0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,   0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, 1.0f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,   0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,   0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,   0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,   0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, -1.0f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,   -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,   -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,   -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,   -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,   -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,   -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,  1.0f, 0.0f,

    //-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    // 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
    // 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    // 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    //-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    //-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,  0.0f, 1.0f
};


// Rail
float segmentLength = 5.0f;
int segmentsPerSide = 3;
int totalStraightSegments = segmentsPerSide * 4;
float railSpacing = 0.4f;
float cornerRadius = 1.0f;

float railVertices[] = {
	-0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f, 1.0f, 1.0f,

	 0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,   0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,   0.0f, 0.0f, -1.0f, 1.0f, 1.0f,

	 0.5f,  0.5f, -0.5f,   0.0f, -0.0f, -1.0f, 1.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,   0.0f, -0.0f, -1.0f, 1.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,   0.0f, -0.0f, -1.0f, 0.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,   -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,   -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,   -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,

	-0.5f,  0.5f,  0.5f,   -1.0f, 0.0f, -0.0f, 0.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,   -1.0f, 0.0f, -0.0f, 0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,   -1.0f, 0.0f, -0.0f, 1.0f, 0.0f,

	0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f, 1.0f, 1.0f,

	0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

	-0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
	0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f, 1.0f, 0.0f,

	0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

    -0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
	0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f, 1.0f, 1.0f,

	0.5f, -0.5f,  0.5f,   -0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,   -0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,   -0.0f, -1.0f, 0.0f, 0.0f, 0.0f
};

float tiesVertices[] = {
    // Front face
    -0.5f, -0.05f,  0.5f,   0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.05f,  0.5f,   0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
     0.5f,  0.05f,  0.5f,   0.0f, 0.0f, 1.0f, 1.0f, 1.0f,

     0.5f,  0.05f,  0.5f,   0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
    -0.5f,  0.05f,  0.5f,   0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
    -0.5f, -0.05f,  0.5f,   0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

    // Back face
    -0.5f, -0.05f, -0.5f,   0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    -0.5f,  0.05f, -0.5f,   0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
     0.5f,  0.05f, -0.5f,   0.0f, 0.0f, -1.0f, 1.0f, 1.0f,

     0.5f,  0.05f, -0.5f,   0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
     0.5f, -0.05f, -0.5f,   0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
    -0.5f, -0.05f, -0.5f,   0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

    // Left face
    -0.5f, -0.05f, -0.5f,   -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    -0.5f, -0.05f,  0.5f,   -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    -0.5f,  0.05f,  0.5f,   -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,

    -0.5f,  0.05f,  0.5f,   -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    -0.5f,  0.05f, -0.5f,   -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    -0.5f, -0.05f, -0.5f,   -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

    // Right face
     0.5f, -0.05f, -0.5f,   1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
     0.5f,  0.05f, -0.5f,   1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
     0.5f,  0.05f,  0.5f,   1.0f, 0.0f, 0.0f, 1.0f, 1.0f,

     0.5f,  0.05f,  0.5f,   1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
     0.5f, -0.05f,  0.5f,   1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
     0.5f, -0.05f, -0.5f,   1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

     // Top face
     -0.5f,  0.05f, -0.5f,   0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
     -0.5f,  0.05f,  0.5f,   0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
      0.5f,  0.05f,  0.5f,   0.0f, 1.0f, 0.0f, 1.0f, 0.0f,

      0.5f,  0.05f,  0.5f,   0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
      0.5f,  0.05f, -0.5f,   0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
     -0.5f,  0.05f, -0.5f,   0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

     // Bottom face
     -0.5f, -0.05f, -0.5f,   0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      0.5f, -0.05f, -0.5f,   0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
      0.5f, -0.05f,  0.5f,   0.0f, -1.0f, 0.0f, 1.0f, 1.0f,

      0.5f, -0.05f,  0.5f,   0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
     -0.5f, -0.05f,  0.5f,   0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
     -0.5f, -0.05f, -0.5f,   0.0f, -1.0f, 0.0f, 0.0f, 0.0f
};

float tileVertices[] = {
    // positions           // tex coords // normals
    -0.5f, 0.0f, -0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
     0.5f, 0.0f, -0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
     0.5f, 0.0f,  0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,

     0.5f, 0.0f,  0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
    -0.5f, 0.0f,  0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
    -0.5f, 0.0f, -0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f
};


int main() {
    // Window Setup
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "My First OpenGL Window", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, processMouseInput);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // Shaders
    Shader shader("shaders/shader.vs", "shaders/shader.frag");
    Shader modelShader("shaders/shader.vs", "shaders/model.frag");
    Shader lightShader("shaders/shader.vs", "shaders/light.frag");

    // Vertex Array Object and Buffer Object Setup
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // Position (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // TexCoord (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int cylinderVertexCount;
    unsigned int cylinderVAO = generateCylinder(10, cylinderVertexCount);

    unsigned int railVAO, railVBO;
    glGenVertexArrays(1, &railVAO);
    glGenBuffers(1, &railVBO);

    glBindVertexArray(railVAO);
    glBindBuffer(GL_ARRAY_BUFFER, railVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(railVertices), railVertices, GL_STATIC_DRAW);

    // Position (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // TexCoord (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

	unsigned int tiesVAO, tiesVBO;
	glGenVertexArrays(1, &tiesVAO);
	glGenBuffers(1, &tiesVBO);

	glBindVertexArray(tiesVAO);
	glBindBuffer(GL_ARRAY_BUFFER, tiesVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tiesVertices), tiesVertices, GL_STATIC_DRAW);

    // Position (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // TexCoord (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int tileVAO, tileVBO;
    glGenVertexArrays(1, &tileVAO);
    glGenBuffers(1, &tileVBO);

    glBindVertexArray(tileVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tileVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tileVertices), tileVertices, GL_STATIC_DRAW);

    // Position (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // TexCoord (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

	// Texture Setup
    unsigned int texture, texture2, texture3, floorTexture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;

    unsigned char* data = stbi_load("assets/container.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    data = stbi_load("assets/rails.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    glGenTextures(1, &texture3);
    glBindTexture(GL_TEXTURE_2D, texture3);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    data = stbi_load("assets/wheels.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    //Floor texture
    glGenTextures(1, &floorTexture);
    glBindTexture(GL_TEXTURE_2D, floorTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    data = stbi_load("assets/floor.jpg", &width, &height, &nrChannels, 0);
    std::cout << "STB reason: " << stbi_failure_reason() << std::endl;
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "Floor loaded: " << width << "x" << height << " channels: " << nrChannels << std::endl;
    }
    else {
        std::cout << "Failed to load floor texture!" << std::endl;
        std::cout << "STB reason: " << stbi_failure_reason() << std::endl;
        return -1;
    }
    stbi_image_free(data);

    Model torchModel("assets/models/torch/model.obj");

	shader.use();
	glUniform1i(glGetUniformLocation(shader.ID, "texture1"), 0);
    shader.setInt("texture1", 0);
    shader.setVec3("lightColor", glm::vec3(0.8f, 0.7f, 0.9f));
    shader.setVec3("lightPos", glm::vec3(2.0f, 10.0f, 2.0f));
    shader.setVec3("viewPos", camera.position);

    glEnable(GL_DEPTH_TEST);

    // Rail Generation
    std::vector<glm::vec3> leftRailPath;
    std::vector<glm::vec3> rightRailPath;
    std::vector<glm::vec3> leftCurvedRailPath;
    std::vector<glm::vec3> rightCurvedRailPath;
	std::vector<glm::vec3> leftBezierRailPath;
	std::vector<glm::vec3> rightBezierRailPath;
    std::vector<std::pair<glm::vec3, glm::vec3>> leftStraightSegments;
    std::vector<std::pair<glm::vec3, glm::vec3>> rightStraightSegments;

    glm::vec3 position(0.0f, 0.0f, 0.0f);
    glm::vec3 direction(1.0f, 0.0f, 0.0f);
    glm::vec3 leftOffset;
    glm::vec3 rightOffset;

    for (int side = 0; side < 4; ++side) {
        glm::vec3 sideDir = glm::normalize(glm::cross(glm::vec3(0, 1, 0), direction));
        leftOffset = sideDir * (-railSpacing / 2.0f);
        rightOffset = sideDir * (railSpacing / 2.0f);

        for (int i = 0; i < segmentsPerSide; ++i) {
			// Bezier Hill
            if (i == 1) {
                std::vector<glm::vec3> leftHill = generateBezierHill(position + leftOffset, direction, segmentLength, 500, 0.5f);
                std::vector<glm::vec3> rightHill = generateBezierHill(position + rightOffset, direction, segmentLength, 500, 0.5f);

                for (size_t j = 0; j < leftHill.size() - 1; ++j) {
                    glm::vec3 l0 = leftHill[j];
                    glm::vec3 l1 = leftHill[j + 1];
                    glm::vec3 r0 = rightHill[j];
                    glm::vec3 r1 = rightHill[j + 1];

                    leftRailPath.push_back(l0);
                    leftRailPath.push_back(l1);
                    rightRailPath.push_back(r0);
                    rightRailPath.push_back(r1);

					leftBezierRailPath.push_back(l0);
					leftBezierRailPath.push_back(l1);
					rightBezierRailPath.push_back(r0);
					rightBezierRailPath.push_back(r1);
                }

                position += direction * segmentLength;
            }
            // Straight
            else {
                glm::vec3 leftStart = position + leftOffset;
                glm::vec3 rightStart = position + rightOffset;

                glm::vec3 leftEnd = leftStart + direction * segmentLength;
                glm::vec3 rightEnd = rightStart + direction * segmentLength;

                leftRailPath.push_back(leftStart);
                leftRailPath.push_back(leftEnd);

                rightRailPath.push_back(rightStart);
                rightRailPath.push_back(rightEnd);

                leftStraightSegments.push_back({ leftStart, leftEnd });
                rightStraightSegments.push_back({ rightStart, rightEnd });

                position += direction * segmentLength;
            }
        }

        // Curve
        glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), direction));
        glm::vec3 curveCenter = position + right * cornerRadius;
        float startAngle = atan2(position.z - curveCenter.z, position.x - curveCenter.x);

        auto leftCurve = generateCurvedRailAroundCenter(curveCenter, cornerRadius, startAngle, -glm::radians(90.0f), 20, -railSpacing / 2.0f);
        auto rightCurve = generateCurvedRailAroundCenter(curveCenter, cornerRadius, startAngle, -glm::radians(90.0f), 20, railSpacing / 2.0f);

        leftRailPath.insert(leftRailPath.end(), rightCurve.begin(), rightCurve.end());
        rightRailPath.insert(rightRailPath.end(), leftCurve.begin(), leftCurve.end());

        leftCurvedRailPath.insert(leftCurvedRailPath.end(), leftCurve.begin(), leftCurve.end());
        rightCurvedRailPath.insert(rightCurvedRailPath.end(), rightCurve.begin(), rightCurve.end());

        position = (leftCurve.back() + rightCurve.back()) * 0.5f;
        direction = glm::normalize(right);
    }

    // Ties
    std::vector<glm::vec3> tiesCurved;
    generateTiesBetweenCurvedRails(leftCurvedRailPath, rightCurvedRailPath, tiesCurved);

    std::vector<glm::vec3> tiesStraight;
    generateTiesBetweenStraightRails(leftStraightSegments, rightStraightSegments, tiesStraight, 10);

	std::vector<glm::vec3> tiesBezier;
	generateTiesBetweenBezierRails(leftBezierRailPath, rightBezierRailPath, tiesBezier);

    // Pre-setup animation
    glm::vec3 cartPosition = calculateMidpoint();
	std::vector<glm::vec3> centerRail = calculateCenterRail(leftRailPath, rightRailPath);
	for (int i = 0; i < centerRail.size(); ++i) {
		std::cout << "Center Rail Point " << i << ": (" << centerRail[i].x << ", " << centerRail[i].y << ", " << centerRail[i].z << ")\n";
	}

	int index = findClosestXZIndex(cartPosition, centerRail);
    float cartSpeed = 2.0f;
    float cartDistance = 0.0f;
    float wheelRotation = 0.0f;

    std::vector<glm::vec3> torchPositions;
    float torchInterval = 5.0f; // place torch every 5 world units
    float distanceAccumulator = 0.0f;
    float lastTorchDistance = 0.0f;

    for (size_t i = 0; i < centerRail.size() - 1; ++i) {
        glm::vec3 from = centerRail[i];
        glm::vec3 to = centerRail[i + 1];
        float segmentDistance = glm::distance(from, to);
        distanceAccumulator += segmentDistance;

        if (distanceAccumulator - lastTorchDistance >= torchInterval) {
            float overshoot = distanceAccumulator - lastTorchDistance - torchInterval;
            float t = 1.0f - overshoot / segmentDistance;
            glm::vec3 pos = glm::mix(from, to, t);

            // offset to side of track
            glm::vec3 dir = glm::normalize(to - from);
            glm::vec3 side = glm::normalize(glm::cross(glm::vec3(0, 1, 0), dir));
            pos += side * 1.0f;
            pos.y += 0.05f;

            torchPositions.push_back(pos);
            lastTorchDistance = distanceAccumulator;
        }
    }

    std::vector<PointLight> torchLights;
    for (const auto& pos : torchPositions) {
        torchLights.push_back({
            pos + glm::vec3(0.0f, 0.8f, 0.0f), 
            glm::vec3(1.0f, 0.6f, 0.2f),
            1.5f                               
            });
    }

    // Main Loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Animation
        cartDistance += cartSpeed * deltaTime;
        float distanceTravelled = cartSpeed * deltaTime;
        wheelRotation = fmod(wheelRotation, 2.0f * glm::pi<float>());

        while (index < static_cast<int>(centerRail.size()) - 1) {
            glm::vec3 from = centerRail[index];
            glm::vec3 to = centerRail[index + 1];
            float segmentLength = glm::distance(from, to);

            if (cartDistance < segmentLength) break;

            cartDistance -= segmentLength;
            ++index;
            if (index >= static_cast<int>(centerRail.size()) - 1) {
                cartDistance = 0.0f;
                index = 0;
            }
        }

        processInput(window);

        // Check if first-person mode is enabled
        if (firstPersonMode) {
            // Lock the camera to the cart position
            glm::vec3 from = centerRail[index];
            glm::vec3 to = centerRail[index + 1];
            float t = cartDistance / glm::distance(from, to);
            glm::vec3 cartPos = glm::mix(from, to, t);
            glm::vec3 direction = glm::normalize(to - from);

            // Update the camera position and orientation based on the cart's position
            camera.position = cartPos + glm::vec3(0.0f, 1.0f, 0.0f); // Adjust Y for camera height
            camera.front = direction;  // Camera follows the cart's forward direction
            camera.updateCameraVectors();  // Update camera vectors based on the new front vector
        }

        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setInt("numPointLights", torchLights.size());

        for (size_t i = 0; i < torchLights.size(); ++i) {
            std::string idx = "pointLights[" + std::to_string(i) + "]";
            shader.setVec3(idx + ".position", torchLights[i].position);
            shader.setVec3(idx + ".color", torchLights[i].color);
            shader.setFloat(idx + ".intensity", torchLights[i].intensity);
        }
        shader.setInt("texture1", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glBindVertexArray(VAO);

        // Camera (view and projection)
        glm::mat4 model = glm::mat4(1.0f);
        shader.setMat4("model", model);
        shader.setMat4("view", camera.viewMatrix());  // The camera view matrix is updated based on position
        shader.setMat4("projection", glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f));

        // Cart rendering logic (this part remains the same as before)
        glm::vec3 from = centerRail[index];
        glm::vec3 to = centerRail[index + 1];
        float t = cartDistance / glm::distance(from, to);

        glm::vec3 cartPos = glm::mix(from, to, t);
        glm::vec3 direction = glm::normalize(to - from);
        float angle = atan2(direction.x, direction.z);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::vec3 xAxis = direction;
        glm::vec3 zAxis = glm::normalize(glm::cross(up, xAxis));
        glm::vec3 yAxis = glm::cross(zAxis, xAxis);

        glm::mat4 rotationMatrix = glm::mat4(1.0f);
        rotationMatrix[0] = glm::vec4(xAxis, 0.0f);
        rotationMatrix[1] = glm::vec4(yAxis, 0.0f);
        rotationMatrix[2] = glm::vec4(zAxis, 0.0f);

        // Render the body of the cart
        model = glm::mat4(1.0f);
        model = glm::translate(model, cartPos + glm::vec3(0.0f, 0.3f, 0.0f));
        model *= rotationMatrix;
        model = glm::scale(model, glm::vec3(1.0f, 0.25f, 0.5f));
        shader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Render the wheels of the cart (this part remains the same as before)
        shader.use();
        shader.setInt("texture1", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture3);

        glBindVertexArray(cylinderVAO);
        for (int i = 0; i < 4; ++i) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cartPos + glm::vec3(0.0f, 0.2f, 0.0f));
            model *= rotationMatrix;
            model = glm::translate(model, wheelPositions[i]);
            model = glm::rotate(model, wheelRotation, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            shader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, cylinderVertexCount);
        }

        shader.use();
        shader.setInt("texture1", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glBindVertexArray(tileVAO);

        for (int x = -50; x < 50; ++x) {
            for (int z = -50; z < 50; ++z) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x, -1.0f, z));
                shader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }
        // Rail and other components rendering (this part remains unchanged)
        shader.use();
        shader.setInt("texture1", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture2);

        drawRail(leftRailPath, shader, railVAO);
        drawRail(rightRailPath, shader, railVAO);
        drawMidRail(centerRail, 0.25f, shader, railVAO);

        drawCurvedTies(tiesCurved, shader, tiesVAO);
        drawStraightTies(tiesStraight, shader, tiesVAO);
        drawStraightTies(tiesBezier, shader, tiesVAO);

        //Lantern by Ian MacGillivray [CC-BY] via Poly Pizza
        glm::mat4 torchModelMatrix = glm::mat4(1.0f);
        torchModelMatrix = glm::translate(torchModelMatrix, glm::vec3(2.0f, 0.0f, 2.0f));
        modelShader.use();
        modelShader.setVec3("lightColor", glm::vec3(1.0f, 0.6f, 0.2f));
        modelShader.setVec3("lightPos", glm::vec3(2.0f, 10.0f, 2.0f));
        modelShader.setVec3("viewPos", camera.position);

        modelShader.setMat4("view", camera.viewMatrix());
        modelShader.setMat4("projection", glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f));

        for (const auto& torchPos : torchPositions) {
            glm::mat4 torchModelMatrix = glm::mat4(1.0f);
            torchModelMatrix = glm::translate(torchModelMatrix, torchPos);
            modelShader.setMat4("model", torchModelMatrix);
            torchModel.Draw(modelShader);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboardInput(FORWARD, deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboardInput(BACKWARD, deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboardInput(LEFT, deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboardInput(RIGHT, deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.processKeyboardInput(UP, deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.processKeyboardInput(DOWN, deltaTime);
	else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		firstPersonMode = !firstPersonMode;
	//else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
	//	camera.position = glm::vec3(0.0f, 0.0f, 0.0f);
	//else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	//	camera.position = glm::vec3(0.0f, 1.0f, 0.0f);
}

void processMouseInput(GLFWwindow* window, double x, double y) {
    if (firstMouse) {
        lastX = static_cast<float>(x);
        lastY = static_cast<float>(y);
        firstMouse = false;
    }

    float xOffset = static_cast<float>(x) - lastX;
    float yOffset = lastY - static_cast<float>(y);

    lastX = static_cast<float>(x);
    lastY = static_cast<float>(y);

    camera.processMouseInput(xOffset, yOffset);
}

int generateCylinder(int segments, unsigned int& vertexCount) {
    std::vector<float> vertices;
    float radius = 0.1f;
    float height = 0.05f;

    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * glm::pi<float>() * i / segments;
        float x = cos(angle) * radius;
        float z = sin(angle) * radius;
        float u = static_cast<float>(i) / segments; // u from 0 to 1

        // Top vertex
        vertices.push_back(x);
        vertices.push_back(height / 2.0f);
        vertices.push_back(z);
        vertices.push_back(u);
        vertices.push_back(0.0f); // v = 0 for top

        // Bottom vertex
        vertices.push_back(x);
        vertices.push_back(-height / 2.0f);
        vertices.push_back(z);
        vertices.push_back(u);
        vertices.push_back(1.0f); // v = 1 for bottom
    }

    vertexCount = static_cast<unsigned int>(vertices.size() / 5);

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return VAO;
}


std::vector<glm::vec3> generateCurvedRailAroundCenter(glm::vec3 center, float radius, float startAngle, float angle, int numSegments, float offsetDistance) {
    std::vector<glm::vec3> railPath;

    for (int i = 0; i < numSegments; ++i) {
        float t = (float)i / (float)(numSegments - 1);
        float theta = startAngle + angle * t;

        glm::vec3 localPos(radius * cos(theta), 0.0f, radius * sin(theta));
        glm::vec3 worldPos = center + localPos;
        glm::vec3 tangent(-sin(theta), 0.0f, cos(theta));
        glm::vec3 side = glm::normalize(glm::cross(glm::vec3(0, 1, 0), tangent));
        glm::vec3 offset = side * offsetDistance;

        railPath.push_back(worldPos + offset);
    }

    return railPath;
}

void generateTiesBetweenCurvedRails(const std::vector<glm::vec3>& leftRailPath, const std::vector<glm::vec3>& rightRailPath, std::vector<glm::vec3>& tiePositions) {
    for (size_t i = 0; i < leftRailPath.size(); ++i) {
        glm::vec3 left = leftRailPath[i];
        glm::vec3 right = rightRailPath[i];

        tiePositions.push_back(left);
        tiePositions.push_back(right);
    }
}

void generateTiesBetweenStraightRails(const std::vector<std::pair<glm::vec3, glm::vec3>>& leftSegments, const std::vector<std::pair<glm::vec3, glm::vec3>>& rightSegments, std::vector<glm::vec3>& ties, int numTies) {
    for (size_t i = 0; i < leftSegments.size(); ++i) {
        glm::vec3 leftStart = leftSegments[i].first;
        glm::vec3 leftEnd = leftSegments[i].second;

        glm::vec3 rightStart = rightSegments[i].first;
        glm::vec3 rightEnd = rightSegments[i].second;

        glm::vec3 leftStep = (leftEnd - leftStart) / float(numTies - 1);
        glm::vec3 rightStep = (rightEnd - rightStart) / float(numTies - 1);

        for (int j = 0; j < numTies; ++j) {
            glm::vec3 leftPos = leftStart + leftStep * float(j);
            glm::vec3 rightPos = rightStart + rightStep * float(j);

            ties.push_back(leftPos);
            ties.push_back(rightPos);
        }
    }
}

void generateTiesBetweenBezierRails(const std::vector<glm::vec3>& leftRailPath, const std::vector<glm::vec3>& rightRailPath, std::vector<glm::vec3>& tiePositions) {
    for (size_t i = 2; i < leftRailPath.size(); i += 50) {
        glm::vec3 left = leftRailPath[i];
        glm::vec3 right = rightRailPath[i];

        tiePositions.push_back(left);
        tiePositions.push_back(right);
    }
}

void drawRail(const std::vector<glm::vec3>& path, const Shader& shader, unsigned int VAO) {
    for (size_t i = 0; i < path.size() - 1; ++i) {
        glm::vec3 p0 = path[i];
        glm::vec3 p1 = path[i + 1];
        glm::vec3 direction = glm::normalize(p1 - p0);
        float length = glm::distance(p0, p1);

        glm::mat4 model = glm::mat4(1.0f);
        glm::vec3 midpoint = (p0 + p1) / 2.0f;
        model = glm::translate(model, midpoint);

        float angle = atan2(direction.z, direction.x);
        model = glm::rotate(model, -angle, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(length, 0.05f, 0.05f));

        shader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

void drawCurvedTies(const std::vector<glm::vec3>& tiePositions, const Shader& shader, unsigned int VAO) {
    for (size_t i = 0; i < tiePositions.size(); i += 10) {
        glm::vec3 left = tiePositions[i];
        glm::vec3 right = tiePositions[i + 1];
        glm::vec3 tieDirection = right - left;
        glm::vec3 tiePosition = (left + right) / 2.0f;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, tiePosition);

        glm::vec3 curveDirection = glm::normalize(tieDirection);
        glm::vec3 referenceDirection = glm::vec3(1.0f, 0.0f, 0.0f);

        float angle = glm::acos(glm::dot(curveDirection, referenceDirection));

        glm::vec3 axisOfRotation = glm::normalize(glm::cross(referenceDirection, curveDirection));
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, axisOfRotation);

        model = model * rotationMatrix;
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.05f, glm::length(tieDirection)));
        shader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

void drawStraightTies(const std::vector<glm::vec3>& tiePositions, const Shader& shader, unsigned int VAO) {
    for (size_t i = 0; i + 1 < tiePositions.size(); i += 2) {
        glm::vec3 left = tiePositions[i];
        glm::vec3 right = tiePositions[i + 1];
        glm::vec3 tieDirection = right - left;
        glm::vec3 tiePosition = (left + right) * 0.5f;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, tiePosition);

        glm::vec3 direction = glm::normalize(tieDirection);
        glm::vec3 reference = glm::vec3(1.0f, 0.0f, 0.0f);

        float angle = glm::acos(glm::clamp(glm::dot(reference, direction), -1.0f, 1.0f));
        glm::vec3 axis = glm::cross(reference, direction);
        if (glm::length(axis) > 0.0001f) {
            model = glm::rotate(model, angle, glm::normalize(axis));
        }

        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.05f, glm::length(tieDirection)));

        shader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

void drawBezierTies(const std::vector<glm::vec3>& tiePositions, const Shader& shader, unsigned int VAO) {
    for (size_t i = 0; i < tiePositions.size(); i += 10) {
        glm::vec3 left = tiePositions[i];
        glm::vec3 right = tiePositions[i + 1];
        glm::vec3 tieDirection = right - left;
        glm::vec3 tiePosition = (left + right) / 2.0f;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, tiePosition);

        glm::vec3 curveDirection = glm::normalize(tieDirection);
        glm::vec3 referenceDirection = glm::vec3(1.0f, 0.0f, 0.0f);

        // Special case for the Z-axis
        if (glm::abs(curveDirection.z) > 0.99f) {
            referenceDirection = glm::vec3(0.0f, 1.0f, 0.0f);  // Use the Y-axis for rotation
        }

        // Calculate the angle and axis of rotation
        float angle = glm::acos(glm::dot(curveDirection, referenceDirection));
        glm::vec3 axisOfRotation = glm::normalize(glm::cross(referenceDirection, curveDirection));

        if (glm::length(axisOfRotation) < 0.01f) {
            axisOfRotation = glm::vec3(0.0f, 1.0f, 0.0f);  // Default to Y-axis if the axis is near zero
        }

        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, axisOfRotation);

        model = model * rotationMatrix;
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.05f, glm::length(tieDirection)));
        shader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}


glm::vec3 calculateMidpoint() {
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();

    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();
    float maxZ = std::numeric_limits<float>::lowest();

    for (int i = 0; i < sizeof(cubeVertices) / sizeof(float); i += 3) {
        float x = cubeVertices[i];
        float y = cubeVertices[i + 1];
        float z = cubeVertices[i + 2];

        minX = std::min(minX, x);
        minY = std::min(minY, y);
        minZ = std::min(minZ, z);

        maxX = std::max(maxX, x);
        maxY = std::max(maxY, y);
        maxZ = std::max(maxZ, z);
    }

    float midX = (minX + maxX) / 2.0f;
    float midY = (minY + maxY) / 2.0f;
    float midZ = (minZ + maxZ) / 2.0f;

	std::cout << "Midpoint: (" << midX << ", " << midY << ", " << midZ << ")\n";

    return {midX, midY, midZ};
}

std::vector<glm::vec3> calculateCenterRail(const std::vector<glm::vec3>& leftRail, const std::vector<glm::vec3>& rightRail) {
    std::vector<glm::vec3> centerRail;

    if (leftRail.size() != rightRail.size()) {
        std::cerr << "Error: Rail vectors must be the same size.\n";
        return centerRail;
    }

    for (size_t i = 0; i < leftRail.size(); ++i) {
        glm::vec3 midpoint = (leftRail[i] + rightRail[i]) * 0.5f;
        centerRail.push_back(midpoint);
    }

    return centerRail;
}

int findClosestXZIndex(const glm::vec3& centerPoint, const std::vector<glm::vec3>& centerRail) {
    float minDistSq = std::numeric_limits<float>::max();
    int closestIndex = -1;

    for (size_t i = 0; i < centerRail.size(); ++i) {
        float dx = centerRail[i].x - centerPoint.x;
        float dz = centerRail[i].z - centerPoint.z;
        float distSq = dx * dx + dz * dz;

        if (distSq < minDistSq) {
            minDistSq = distSq;
            closestIndex = static_cast<int>(i);
        }
    }

    return closestIndex;
}

std::vector<glm::vec3> generateBezierHill(glm::vec3 start, glm::vec3 direction, float segmentLength, int numSegments, float hillHeight) {
    std::vector<glm::vec3> hillPath;

    glm::vec3 p0 = start;
    glm::vec3 p3 = start + direction * segmentLength;

    glm::vec3 up(0.0f, hillHeight, 0.0f);

    glm::vec3 p1 = p0 + direction * (segmentLength / 3.0f) + up;
    glm::vec3 p2 = p0 + direction * (2.0f * segmentLength / 3.0f) + up;

    for (int i = 0; i < numSegments; ++i) {
        float t = (float)i / (float)(numSegments - 1);

        glm::vec3 point =
            static_cast<float>(std::pow(1 - t, 3)) * p0 +
            3.0f * static_cast<float>(std::pow(1 - t, 2)) * t * p1 +
            3.0f * (1 - t) * static_cast<float>(std::pow(t, 2)) * p2 +
            static_cast<float>(std::pow(t, 3)) * p3;

        hillPath.push_back(point);
    }

    return hillPath;
}

void drawMidRail(const std::vector<glm::vec3>& path, float y, const Shader& shader, unsigned int VAO) {
    for (size_t i = 0; i < path.size() - 1; ++i) {
        glm::vec3 p0 = path[i];
        glm::vec3 p1 = path[i + 1];
        glm::vec3 direction = glm::normalize(p1 - p0);
        float length = glm::distance(p0, p1);

        glm::mat4 model = glm::mat4(1.0f);
        glm::vec3 midpoint = (p0 + p1) / 2.0f;
        //midpoint.y += y;  // <-- Offset the Y position
        model = glm::translate(model, midpoint);

        float angle = atan2(direction.z, direction.x);
        model = glm::rotate(model, -angle, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(length, 0.05f, 0.05f));

        shader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

