#version 330 core

out vec4 FragColor;

uniform vec4 fragmentColour;

void main()
{
	FragColor = fragmentColour;
}