#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 viewMatrix = mat4(1.0);
uniform mat4 projectionMatrix = mat4(1.0);
uniform mat4 worldMatrix = mat4(1.0);
uniform mat4 transformMatrix = mat4(1.0);

void main()
{
	mat4 mvp = projectionMatrix * viewMatrix * worldMatrix * transformMatrix;
	gl_Position =  mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}