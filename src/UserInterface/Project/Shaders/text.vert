#version 150 compatibility

in vec2 position;
//in vec2 tex;
//out vec4 pos;
//out vec2 fTexCoord;

void main(void) {
   gl_Position = gl_ModelViewMatrix * vec4(position, 0.5, 1);
   gl_TexCoord[0]=gl_MultiTexCoord0; 
   //gl_TexCoord[0] = vec4(tex, 0, 0);
   gl_FrontColor = gl_Color;
}

