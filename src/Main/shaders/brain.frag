#version 450

layout (std140, binding = 0) uniform Time {
    float time;
} time;

layout (std140, binding = 1) uniform Camera {
    mat4 viewMatrix;
    mat4 VP;
    mat4 rayDirMatrix;
    vec3 position;
    vec3 viewDir;
    uint displayMode;
} camera;

layout (binding = 2) uniform sampler3D agySampler;
layout (binding = 3) uniform sampler2D matcapSampler;

layout (location = 0) in vec2 uv;
layout (location = 1) in vec3 rayDirection;

layout (location = 0) out vec4 presented;


#define EPS 1.0e-5
#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38
#define DBL_MAX 1.7976931348623158e+308
#define DBL_MIN 2.2250738585072014e-308



float SurfaceIntersection (vec3 rayStart, vec3 rayDir, vec3 surfacePoint, vec3 surfaceNormal)
{
    float t = (dot (surfaceNormal, surfacePoint) - dot (surfaceNormal, rayStart)) / dot (rayDir, surfaceNormal);
    return t;
}


vec2 BoxIntersection (vec3 rayStart, vec3 rayDir, vec3 middle, float radius)
{
    vec3 normalTop    = vec3 (0, 0, +1);
    vec3 normalBottom = vec3 (0, 0, -1);
    vec3 normalRight  = vec3 (0, +1, 0);
    vec3 normalLeft   = vec3 (0, -1, 0);
    vec3 normalFront  = vec3 (+1, 0, 0);
    vec3 normalBack   = vec3 (-1, 0, 0);

    float top    = max (0, SurfaceIntersection (rayStart, rayDir, middle + normalTop    * radius, normalTop));
    float bottom = max (0, SurfaceIntersection (rayStart, rayDir, middle + normalBottom * radius, normalBottom));
    float right  = max (0, SurfaceIntersection (rayStart, rayDir, middle + normalRight  * radius, normalRight));
    float left   = max (0, SurfaceIntersection (rayStart, rayDir, middle + normalLeft   * radius, normalLeft));
    float front  = max (0, SurfaceIntersection (rayStart, rayDir, middle + normalFront  * radius, normalFront));
    float back   = max (0, SurfaceIntersection (rayStart, rayDir, middle + normalBack   * radius, normalBack));

    vec3 topIntersectionPoint    = rayStart + rayDir * top;
    vec3 bottomIntersectionPoint = rayStart + rayDir * bottom;
    vec3 rightIntersectionPoint  = rayStart + rayDir * right;
    vec3 leftIntersectionPoint   = rayStart + rayDir * left;
    vec3 frontIntersectionPoint  = rayStart + rayDir * front;
    vec3 backIntersectionPoint   = rayStart + rayDir * back;

    // find first intersection
    float belepT_X = min (left, right);
    float belepT_Y = min (front, back);
    float belepT_Z = min (bottom, top);
    float belepT   = max (belepT_X, max (belepT_Y, belepT_Z));

    float kilepT_X = max (left, right);
    float kilepT_Y = max (front, back);
    float kilepT_Z = max (bottom, top);
    float kilepT   = min (kilepT_X, min (kilepT_Y, kilepT_Z));
    
    if (belepT > kilepT) {
        return vec2 (-1.f, -1.f);
    }

    return vec2 (belepT, kilepT);
}


vec3 GetNormalAt (vec3 rayPos) 
{
    float delta = 1.f/256.f * 5.f;
    vec3 gradient = vec3 (
        (texture (agySampler, vec3 (rayPos.x + delta, rayPos.y, rayPos.z)).r - texture (agySampler, vec3 (rayPos.x - delta, rayPos.y, rayPos.z)).r) / (2 * delta),
        (texture (agySampler, vec3 (rayPos.x, rayPos.y + delta, rayPos.z)).r - texture (agySampler, vec3 (rayPos.x, rayPos.y - delta, rayPos.z)).r) / (2 * delta),
        (texture (agySampler, vec3 (rayPos.x, rayPos.y, rayPos.z + delta)).r - texture (agySampler, vec3 (rayPos.x, rayPos.y, rayPos.z - delta)).r) / (2 * delta)
    );
    return normalize (gradient.rgb);
}


void main ()
{
    presented = vec4 (vec3 (0), 1);

    const int DP_SZINTFELULET = 2;
    const int DP_MATCAP = 3;
    const int DP_HAGYMA = 4;
    const int DP_ARNYEK = 5;

    const vec3 normRayDir = normalize (rayDirection);

    const vec2 boxT = BoxIntersection (camera.position, normRayDir, vec3 (0.5f, 0.5f, 0.5f), 0.5f);
    const bool boxHit = (boxT.x >= 0.f && boxT.y >= 0.f);

    const int   steps = 512;
    const float rayStep = abs (boxT.y - boxT.x) / steps;
    
    float rayT = boxT.x;
    vec3  rayPos  = camera.position + normRayDir * rayT;

    float insideBrain = 0.002f;
    if (camera.displayMode == DP_HAGYMA) {
        insideBrain = 0.09f;
    }

    const float minConsideredValue = 0.01f;

    int hagymaSampledCount = 0;
    const int maxHagymaSampleCount = 6;
      
    const vec3 lightDir = normalize (vec3 (0, 0, -1));
    
    if (boxHit) {

        float volume = 0.f;
        int brainHitCount = 0;

        vec4 hagyma = vec4 (vec3 (1), 0.0);

        for (int i = 0; i < steps; ++i) {
            float sampled = texture (agySampler, rayPos).r;
            if (sampled > minConsideredValue) {
                brainHitCount++;
                volume += sampled / steps;
                
                if (camera.displayMode == DP_HAGYMA) {
                    if (volume > (hagymaSampledCount) * insideBrain) {
                        hagyma.w = max (hagyma.w, 1.f-max (0, dot (GetNormalAt (rayPos), normalize (rayPos - camera.position))));
                        hagymaSampledCount++;
                    }
                }
            }

            bool isHit = false;
            if (camera.displayMode == DP_HAGYMA) {
                isHit = (hagymaSampledCount == maxHagymaSampleCount);
            } else  if (camera.displayMode == DP_ARNYEK) {
                isHit = (brainHitCount > 0);
            } else {
                isHit = (volume > insideBrain);
            }

            if (isHit) {
                break;
            }
            
            rayT += rayStep;
            rayPos = camera.position + normRayDir * rayT;
        }

        if (brainHitCount > 0) {
            const vec3 gradient = GetNormalAt (rayPos);
            const vec3 viewDir = normalize (rayPos - camera.position);
            const float angle = max (0.5, dot (gradient, lightDir));

            const vec3 lightColor = vec3 (255, 241, 224)/255.f;
            const vec3 objectColor = vec3 (217,165,178)/255.f;

            const vec3 ambient = 0.1f * lightColor;
  	
            const float diff = max(dot(gradient, lightDir), 0.08);
            const vec3 diffuse = diff * lightColor;
    
            const vec3 reflectDir = reflect(-lightDir, gradient);  
            const float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8);
            const vec3 specular = 0.5f * spec * lightColor;  

            const vec3 phongColor = (ambient + diffuse + specular) * objectColor;

        

            if (camera.displayMode == DP_SZINTFELULET) {
                presented = vec4 (phongColor, 1.f);
            
            } else if (camera.displayMode == DP_MATCAP) {
                const vec4 n              = vec4 (gradient, 1) * camera.viewMatrix;
                const vec3 matcapTexture  = texture (matcapSampler, (n.rg + 1) * 0.5).rgb;
                presented = vec4 (matcapTexture.rgb, 1.f);
            
            } else if (camera.displayMode == DP_HAGYMA) {
                presented = hagyma;
            
            } else if (camera.displayMode == DP_ARNYEK) {

                // second ray to light dir
                const vec3 ray2StartPos = rayPos;
                const vec2 box2T = BoxIntersection (ray2StartPos, lightDir, vec3 (0.5f, 0.5f, 0.5f), 0.5f);

                const int shadowSamples = 256;
                const float ray2StepSize = box2T.y / shadowSamples; 
                
                vec3 ray2Pos = ray2StartPos;
                float ray2T = 0.0f;

                float ray2AccumulatedVolume = 0;

                for (int i = 0; i < shadowSamples; ++i) {
                
                    float sampled = texture (agySampler, ray2Pos).r;
                    if (sampled > minConsideredValue) {
                        ray2AccumulatedVolume += sampled / shadowSamples;
                    }

                    ray2T += ray2StepSize;
                    ray2Pos = ray2StartPos + lightDir * ray2T;
                }

                if (ray2AccumulatedVolume > 0.001f) {
                    presented = vec4 (vec3 (0), 0.2f);
                } else {
                    presented = vec4 (vec3 (phongColor), 1.f);
                }

            } else {
                // error
                presented = vec4 (vec3 (1, 0, 0), 1);
            }
    
            //presented = vec4 (gradient.rgb * 0.5 + 0.5, 1.f);
        }
    }
    //presented = vec4 (texture (matcapSampler, uv).rgb, 1);

    //presented = vec4 (vec3 (bt.x), 1);

    //presented = vec4 (texture (agySampler, vec3 (uv.y, uv.x, fract(time.time*0.1f))).rrr, 1);
}