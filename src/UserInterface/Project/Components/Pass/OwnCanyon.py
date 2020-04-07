import Gears as gears
from .. import * 
import traceback
from .Base import *

class OwnCanyon(Base) : 

    def __init__(self, **args):
        super().__init__(**args)

    def boot(self,
            *,
            duration        : 'Stimulus time in frames (unless superseded by duration_s).'
                            =   1,
            duration_s      : 'Stimulus time in seconds (takes precendece over duration given in frames).'
                            =   0,
            name            : 'Stimulus name to display in sequence overview plot.'
                            = 'Luna',
            imagePath0      : 'Image path.'
                            = './Project/Media/dunes-bump.jpg',
            imagePath1      : 'Image path.'
                            = './Project/Media/tex07.jpg',
            imagePath2      : 'Image path.'
                            = './Project/Media/tex16.png',
            imagePath3      : 'Image path.'
                            = './Project/Media/tex06.jpg'
            ):
        self.name                =      name
        self.duration            =      duration
        sequence = self.getSequence()
        stimulus = self.getStimulus()
        self.enableColorMode();
        self.duration = duration
        if(duration == 0):
            self.duration = stimulus.getDuration()
        if(duration_s != 0):
            self.duration = int(duration_s // sequence.getFrameInterval_s() + 1)

        self.setShaderImage('iChannel0', imagePath0)
        self.setShaderImage('iChannel1', imagePath1)
        self.setShaderImage('iChannel2', imagePath2)
        self.setShaderImage('iChannel3', imagePath3)
     
        self.setShaderVector('iResolution', x=sequence.field_width_px, y=sequence.field_width_px)

        self.stimulusGeneratorShaderSource = """
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;
uniform sampler2D iChannel3;

float snoise(vec3 r) {
	vec3 s = vec3(7502, 22777, 4767);
	float f = 0.0;
	for(int i=0; i<16; i++) {
		f += sin( dot(s, r) / 65536.0);
		s = mod(s, 32768.0) * 2.0 + floor(s / 32768.0);
	}
	return f / 32.0 + 0.5;
}


vec3 snoiseGrad(vec3 r) {
	vec3 s = vec3(7502, 22777, 4767);
	vec3 f = vec3(0.0, 0.0, 0.0);
	for(int i=0; i<16; i++) {
		f += cos( dot(s, r) / 65536.0) * s;
		s = mod(s, 32768.0) * 2.0 + floor(s / 32768.0);
	}
	return f / 65536.0;
}

/*float snoise(vec3 r)
{
	uint x = 0x0625DF73u;
	uint y = 0xD1B84B45u;
	uint z = 0x152AD8D0u;
	float f = 0.0;
	for(uint i=0u; i<32u; i++)
	{
		vec3 s = vec3(x/float(0xffffffff), y/float(0xffffffff), z/float(0xffffffff));
		f += sin(dot(s, r));
		x = x << 1 | x >> 31;
		y = y << 1 | y >> 31;
		z = z << 1 | z >> 31;
	}
	return f / 64.0 + 0.5;	
}

vec3 snoiseGrad(vec3 r)
{
	uint x = 0x0625DF73u;
	uint y = 0xD1B84B45u;
	uint z = 0x152AD8D0u;
	vec3 f = vec3(0, 0, 0);
	for(uint i=0u; i<32u; i++)
	{
		vec3 s = vec3(x/float(0xffffffff), y/float(0xffffffff), z/float(0xffffffff));
		f += s * cos(dot(s, r));
		x = x << 1 | x >> 31;
		y = y << 1 | y >> 31;
		z = z << 1 | z >> 31;
	}
	return f * (1.0 / 64.0);
}*/

float noise1( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
	f = f*f*(3.0-2.0*f);
	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z);
	vec2 rg1 = texture2D( iChannel2, (uv+ vec2(0.5,0.5))/256.0, -100.0 ).yx;
	vec2 rg2 = texture2D( iChannel2, (uv+ vec2(1.5,0.5))/256.0, -100.0 ).yx;
	vec2 rg3 = texture2D( iChannel2, (uv+ vec2(0.5,1.5))/256.0, -100.0 ).yx;
	vec2 rg4 = texture2D( iChannel2, (uv+ vec2(1.5,1.5))/256.0, -100.0 ).yx;
	vec2 rg = mix( mix(rg1,rg2,f.x), mix(rg3,rg4,f.x), f.y );
	return mix( rg.x, rg.y, f.z );
}

//-----------------------------------------------------------------------------------
const mat3 m = mat3( 0.00,  0.80,  0.60,
                    -0.80,  0.36, -0.48,
                    -0.60, -0.48,  0.64 );

float displacement( vec3 p )
{
    float f;
    f  = 0.5000*snoise( p ); p = m*p*2.02;
    f += 0.2500*snoise( p ); p = m*p*2.03;
    f += 0.1250*snoise( p ); p = m*p*2.01;
    f += 0.0625*snoise( p ); 
    return f;
}

vec4 texcube( sampler2D sam, in vec3 p, in vec3 n )
{
	vec4 x = texture2D( sam, p.yz );
	vec4 y = texture2D( sam, p.zx );
	vec4 z = texture2D( sam, p.xy );
	return (x*abs(n.x) + y*abs(n.y) + z*abs(n.z))/(abs(n.x)+abs(n.y)+abs(n.z));
}


//-----------------------------------------------------------------------------------

float terrain( in vec2 q )
{
	float th = smoothstep( 0.0, 0.7, texture2D( iChannel0, 0.005*q, -100.0 ).x );
    float rr = 0.0;//smoothstep( 0.1, 0.5, texture2D( iChannel1, 2.0*0.03*q, -100.0 ).x );
	float h = 1.9;
	h += (1.0-0.6*rr)*(1.5-1.0*th) * 0.2*(1.0 /*-texture2D( iChannel0, 0.093*q, -100.0 ).x*/ );
	h += th*7.0;
    h += 0.3*rr;
    return -h;

}

vec4 map( in vec3 p )
{
	float h = terrain( p.xz );
	float dis = snoise(p*20.0);//displacement( 0.25*p*vec3(1.0,4.0,1.0) );
	dis *= 0.1;
	return vec4( ( dis + p.y-h)*0.25, p.x, h, 0.0 );
}

vec4 intersect( in vec3 ro, in vec3 rd, in float tmax )
{
    float t = 0.1;
    vec3 res = vec3(0.0);
    for( int i=0; i<256; i++ )
    {
	    vec4 tmp = map( ro+rd*t );
        res = tmp.ywz;
        t += tmp.x;
        if( tmp.x<(0.001*t) || t>tmax ) break;
    }

    return vec4( t, res );
}

vec3 calcNormal( in vec3 pos, in float t )
{
    vec2 eps = vec2( 0.005*t, 0.0 );
	return normalize( vec3(
           map(pos+eps.xyy).x - map(pos-eps.xyy).x,
           map(pos+eps.yxy).x - map(pos-eps.yxy).x,
           map(pos+eps.yyx).x - map(pos-eps.yyx).x ) );
}

float softshadow( in vec3 ro, in vec3 rd, float mint, float k )
{
    float res = 1.0;
    float t = mint;
    for( int i=0; i<50; i++ )
    {
        float h = map(ro + rd*t).x;
        res = min( res, k*h/t );
		t += clamp( h, 0.5, 1.0 );
		if( h<0.001 ) break;
    }
    return clamp(res,0.0,1.0);
}

// Oren-Nayar
float Diffuse( in vec3 l, in vec3 n, in vec3 v, float r )
{
	
    float r2 = r*r;
    float a = 1.0 - 0.5*(r2/(r2+0.57));
    float b = 0.45*(r2/(r2+0.09));

    float nl = dot(n, l);
    float nv = dot(n, v);

    float ga = dot(v-n*nv,n-n*nl);

	return max(0.0,nl) * (a + b*max(0.0,ga) * sqrt((1.0-nv*nv)*(1.0-nl*nl)) / max(nl, nv));
}

vec3 cpath( float t )
{
	vec3 pos = vec3( 0.0, 0.0, 95.0 + t );
	
	float a = smoothstep(5.0,20.0,t);
	pos.xz += a*150.0 * cos( vec2(5.0,6.0) + 1.0*0.01*t );
	pos.xz -= a*150.0 * cos( vec2(5.0,6.0) );
	pos.xz += a* 50.0 * cos( vec2(0.0,3.5) + 6.0*0.01*t );
	pos.xz -= a* 50.0 * cos( vec2(0.0,3.5) );

	return pos;
}

mat3 setCamera( in vec3 ro, in vec3 ta, float cr )
{
	vec3 cw = normalize(ta-ro);
	vec3 cp = vec3(sin(cr), cos(cr),0.0);
	vec3 cu = normalize( cross(cw,cp) );
	vec3 cv = normalize( cross(cu,cw) );
    return mat3( cu, cv, cw );
}
out vec4 fragColor;

void main(  )
{
    vec2 q = gl_FragCoord.xy / iResolution.xy;
	vec2 p = -1.0 + 2.0*q;
	p.x *= iResolution.x / iResolution.y;

    vec2 m = vec2(0.0);
	
    //-----------------------------------------------------
    // camera
    //-----------------------------------------------------

	float an = 0.5*(time-5.0);// + 12.0*(m.x-0.5);
	vec3 ro = cpath( an + 0.0 );
	vec3 ta = cpath( an + 10.0 *1.0);
	ta = mix( ro + vec3(0.0,0.0,1.0), ta, smoothstep(5.0,25.0,an) );
    ro.y = 0.0 - 0.5;
	ta.y = ro.y - 0.1;
	ta.xy += step(0.01,m.x)*(m.xy-0.5)*4.0*vec2(-1.0,1.0);
	float rl = -0.1*cos(0.05*6.2831*an);
    // camera to world transform    
    mat3 cam = setCamera( ro, ta, 0/*rl*/ );
    
    // ray
	vec3 rd = normalize( cam * vec3(p.xy, 2.0) );

    //-----------------------------------------------------
	// render
    //-----------------------------------------------------

    vec3 col = vec3(0.0, 0.7, 1.0);

	// raymarch
    float tmax = 120.0;
    
    // bounding plane    
    float bt = (0.0-ro.y)/rd.y; 
	if( bt>0.0 ) tmax = min( tmax, bt );
        
    vec4 tmat = intersect( ro, rd, tmax);
    if( tmat.x<tmax )
    {
        // geometry
        vec3 pos = ro + tmat.x*rd;
        vec3 nor = calcNormal( pos, tmat.x );

        col = vec3(1, 1, 1) * Diffuse(normalize(vec3(1.0, 4.0, 1.0)), nor, -rd, 1.0);
	}


	fragColor = vec4( col, 1.0 );
}
		"""





