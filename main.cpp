#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <camera.h>
#include <vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void processMouseInput(GLFWwindow* window, double x, double y);
void drawRail(const std::vector<glm::vec3>& path, unsigned int shaderProgram, unsigned int VAO, int modelLoc);
void drawCurvedTies(const std::vector<glm::vec3>& ties, unsigned int shaderProgram, unsigned int VAO, int modelLoc);
void drawStraightTies(const std::vector<glm::vec3>& ties, unsigned int shaderProgram, unsigned int VAO, int modelLoc);
int generateCylinder(int segments, unsigned int& vertexCount);
void generateTiesBetweenCurvedRails(const std::vector<glm::vec3>& leftRailPath, const std::vector<glm::vec3>& rightRailPath, std::vector<glm::vec3>& ties);
void generateTiesBetweenStraightRails(const std::vector<std::pair<glm::vec3, glm::vec3>>& leftSegments, const std::vector<std::pair<glm::vec3, glm::vec3>>& rightSegments, std::vector<glm::vec3>& ties, int numTies);
std::vector<glm::vec3> generateCurvedRailAroundCenter(glm::vec3 center, float radius, float startAngle, float angle, int numSegments, float offsetDistance);

// Shaders
const char* vertexCameraShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;

    out vec2 TexCoord;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(aPos, 1.0f);
        TexCoord = vec2(aTexCoord.x, aTexCoord.y);
    }
)";

const char* fragmentCameraShaderSource = R"(
    #version 330 core
    out vec4 FragColor;

    void main()
    {
	    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
    }
)";

// Window
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Camera
Camera camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
bool firstMouse = true;
float lastX = 800 / 2.0f;
float lastY = 600 / 2.0f;

// Wheels
float wheelRadius = 0.1f;
float railTopY = -0.15f;
float wheelCenterY = railTopY + wheelRadius + 0.15f;

glm::vec3 wheelPositions[4] = {
    { 1.55f, wheelCenterY, -0.2f},
    { 2.45f, wheelCenterY, -0.2f},
    { 1.55f, wheelCenterY,  0.2f},
    { 2.45f, wheelCenterY,  0.2f}
};

// Body
float cubeVertices[] = {
     1.5f, -0.5f, -0.5f,
     2.5f, -0.5f, -0.5f,
     2.5f,  0.5f, -0.5f,
     2.5f,  0.5f, -0.5f,
     1.5f,  0.5f, -0.5f,
     1.5f, -0.5f, -0.5f,

     1.5f, -0.5f,  0.5f,
     2.5f, -0.5f,  0.5f,
     2.5f,  0.5f,  0.5f,
     2.5f,  0.5f,  0.5f,
     1.5f,  0.5f,  0.5f,
     1.5f, -0.5f,  0.5f,

     1.5f,  0.5f,  0.5f,
     1.5f,  0.5f, -0.5f,
     1.5f, -0.5f, -0.5f,
     1.5f, -0.5f, -0.5f,
     1.5f, -0.5f,  0.5f,
     1.5f,  0.5f,  0.5f,

     2.5f,  0.5f,  0.5f,
     2.5f,  0.5f, -0.5f,
     2.5f, -0.5f, -0.5f,
     2.5f, -0.5f, -0.5f,
     2.5f, -0.5f,  0.5f,
     2.5f,  0.5f,  0.5f,

     1.5f, -0.5f, -0.5f,
     2.5f, -0.5f, -0.5f,
     2.5f, -0.5f,  0.5f,
     2.5f, -0.5f,  0.5f,
     1.5f, -0.5f,  0.5f,
     1.5f, -0.5f, -0.5f,

     1.5f,  0.5f, -0.5f,
     2.5f,  0.5f, -0.5f,
     2.5f,  0.5f,  0.5f,
     2.5f,  0.5f,  0.5f,
     1.5f,  0.5f,  0.5f,
     1.5f,  0.5f, -0.5f
};

// Rail
float segmentLength = 2.0f;
int segmentsPerSide = 3;
int totalStraightSegments = segmentsPerSide * 4;
float railSpacing = 0.4f;
float cornerRadius = 1.0f;

float railVertices[] = {
    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,

     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,

     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,

    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,

    0.5f, -0.5f, -0.5f,
    0.5f,  0.5f, -0.5f,
    0.5f,  0.5f,  0.5f,

    0.5f,  0.5f,  0.5f,
    0.5f, -0.5f,  0.5f,
    0.5f, -0.5f, -0.5f,

    -0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,

    0.5f,  0.5f,  0.5f,
    0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,

    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, -0.5f,  0.5f,

    0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f
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

	// Shader Setup
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexCameraShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentCameraShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

	// Vertex Array Object and Buffer Object Setup
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int cylinderVertexCount;
    unsigned int cylinderVAO = generateCylinder(30, cylinderVertexCount);

    unsigned int railVAO, railVBO;
    glGenVertexArrays(1, &railVAO);
    glGenBuffers(1, &railVBO);

    glBindVertexArray(railVAO);
    glBindBuffer(GL_ARRAY_BUFFER, railVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(railVertices), railVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glEnable(GL_DEPTH_TEST);

	// Rail Generation
    std::vector<glm::vec3> leftRailPath;
    std::vector<glm::vec3> rightRailPath;
	std::vector<glm::vec3> leftCurvedRailPath;
	std::vector<glm::vec3> rightCurvedRailPath;
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

        // Straight
        for (int i = 0; i < segmentsPerSide; ++i) {
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

        // Curve
        glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), direction));
        glm::vec3 curveCenter = position + right * cornerRadius;
        float startAngle = atan2(position.z - curveCenter.z, position.x - curveCenter.x);

        auto leftCurve = generateCurvedRailAroundCenter(curveCenter, cornerRadius, startAngle, -glm::radians(90.0f), 20, -railSpacing / 2.0f);
        auto rightCurve = generateCurvedRailAroundCenter(curveCenter, cornerRadius, startAngle, -glm::radians(90.0f), 20, railSpacing / 2.0f);

        leftRailPath.insert(leftRailPath.end(), leftCurve.begin(), leftCurve.end());
        rightRailPath.insert(rightRailPath.end(), rightCurve.begin(), rightCurve.end());

		leftCurvedRailPath.insert(leftCurvedRailPath.end(), leftCurve.begin(), leftCurve.end());
		rightCurvedRailPath.insert(rightCurvedRailPath.end(), rightCurve.begin(), rightCurve.end());

        position = (leftCurve.back() + rightCurve.back()) * 0.5f;
        direction = glm::normalize(right);
    }

	// Ties
    std::vector<glm::vec3> tiesCurved;
    generateTiesBetweenCurvedRails(leftCurvedRailPath, rightCurvedRailPath, tiesCurved);

	std::vector<glm::vec3> tiesStraight;
	generateTiesBetweenStraightRails(leftStraightSegments, rightStraightSegments, tiesStraight, 5);

	// Main Loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        int modelLoc = glGetUniformLocation(shaderProgram, "model");
        int viewLoc = glGetUniformLocation(shaderProgram, "view");
        int projLoc = glGetUniformLocation(shaderProgram, "projection");

        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Camera
		glm::mat4 projectionCamera = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
		glm::mat4 viewCamera = camera.viewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewCamera));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionCamera));

        // Body
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.3f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 0.25f, 0.5f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Wheels
        glBindVertexArray(cylinderVAO);

        for (int i = 0; i < 4; ++i) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, wheelPositions[i]);
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, cylinderVertexCount);
        }


        drawRail(leftRailPath, shaderProgram, railVAO, modelLoc);
		drawRail(rightRailPath, shaderProgram, railVAO, modelLoc);

        drawCurvedTies(tiesCurved, shaderProgram, railVAO, modelLoc);
        drawStraightTies(tiesStraight, shaderProgram, railVAO, modelLoc);

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

        vertices.push_back(x);
        vertices.push_back(height / 2.0f);
        vertices.push_back(z);

        vertices.push_back(x);
        vertices.push_back(-height / 2.0f);
        vertices.push_back(z);
    }

    vertexCount = static_cast<unsigned int>(vertices.size() / 3);

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

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

void generateTiesBetweenStraightRails(
    const std::vector<std::pair<glm::vec3, glm::vec3>>& leftSegments,
    const std::vector<std::pair<glm::vec3, glm::vec3>>& rightSegments,
    std::vector<glm::vec3>& ties, int numTies) {

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

void drawRail(const std::vector<glm::vec3>& path, unsigned int shaderProgram, unsigned int VAO, int modelLoc) {
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

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

void drawCurvedTies(const std::vector<glm::vec3>& tiePositions, unsigned int shaderProgram, unsigned int VAO, int modelLoc) {
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

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

void drawStraightTies(const std::vector<glm::vec3>& tiePositions, unsigned int shaderProgram, unsigned int VAO, int modelLoc) {
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

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}