#version 450

layout (std140, binding = 0) uniform Time {
    float time;
} time;

layout (std140, binding = 3) uniform Camera {
    mat4 viewMatrix;
    mat4 VP;
    mat4 rayDirMatrix;
    vec3 position;
    vec3 viewDir;
} camera;

layout (binding = 1) uniform sampler2D agy2dSampler;
layout (binding = 2) uniform sampler3D agySampler;
layout (binding = 4) uniform sampler2D matcapSampler;

layout (location = 0) in vec2 uv;
layout (location = 1) in vec3 rayDirection;

layout (location = 2) out vec4 presented;
layout (location = 0) out vec4 copy[2];




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


bool IsInsideBox (vec3 p, vec3 boxMiddle, float boxRadius)
{
    boxRadius += EPS;
    vec3 delta = p - boxMiddle;
    return abs (delta.x) <= boxRadius && abs (delta.y) <= boxRadius && abs (delta.z) <= boxRadius;
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


float TriangleIntersection (vec3 rayStart, vec3 rayDir, vec3 p1, vec3 p2, vec3 p3)
{
    vec3 normal = normalize (cross (p2 - p1, p3 - p1));

    float t = dot (p1 - rayStart, normal) / dot (rayDir, normal);
    //if (t < 0) {
    //    return -1.f;
    //}

    vec3 P = rayStart + rayDir * t;

    if (dot (cross (p2 - p1, P - p1), normal) > 0 &&
        dot (cross (p3 - p2, P - p2), normal) > 0 &&
        dot (cross (p1 - p3, P - p3), normal) > 0) {

        return t;
    }

    return -1.f;
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
    vec4 result = vec4 (vec3 (uv, 1.f), 1);
    presented = vec4 (texture (agySampler, vec3(uv, mod (time.time, fract (time.time*0.1f))  )).rrr, 1);
    //presented = result;
    //presented = vec4 (normalize (rayDirection) * 0.5 + 0.5, 1);

    vec3 normRayDir = normalize (rayDirection);
    //vec3 rayPos = camera.position;
    //float rayStep = 0.1f;
    //float val = 0.f;
//
//    vec3 sphereCenter = vec3 (0, 0, 0);
//    float sphereRadius = 1.f;
//
//    float minDist = 99999.f;
//    //presented = vec4 (vec3 (distance (rayPos, sphereCenter) - sphereRadius), 1.f);
//
//    for (int i = 0; i < 64; ++i) {
//        float dist = distance (rayPos, sphereCenter) - sphereRadius;
//        if (dist < minDist) {
//            minDist = dist;
//        }
//        //val += texture (agySampler, rayPos).r;
//
//        rayPos += normRayDir * rayStep;
//    }
//
    //presented = vec4 (vec3 (minDist), 1);
    //presented = vec4 (normalize (rayDirection) * 0.5 + 0.5, 1);

    //float t = SurfaceIntersection (camera.position, normRayDir, vec3 (0, 0, -1), vec3 (0, 0, 1));
    //t = TriangleIntersection (camera.position, normRayDir, vec3 (1, 0, 0), vec3 (0, 0, 1), vec3 (0, 1, 0));
    vec2 bt = BoxIntersection (camera.position, normRayDir, vec3 (0.5f, 0.5f, 0.5f), 0.5f);
    bool hit = (bt.x >= 0.f  && bt.x < FLT_MAX && bt.y >= 0.f && bt.y < FLT_MAX);

    presented = vec4 (vec3 (0), 1);

    int   steps = 1024;
    float rayStep = abs (bt.y - bt.x) / steps;
    float rayT = bt.x;
    vec3  rayPos  = camera.position + normRayDir * rayT;

    float insideBrain = 0.01f;
    float minConsideredValue = 0.09f;

    int sampleCount = 0;
    int hagymaSampledCount = 0;
    int maxHagymaSampleCount = 4;
      
    if (hit) {

        vec4 gradient = vec4 (0, 0, 0, 0);
        float volume = 0.f;

        vec4 hagyma = vec4 (vec3 (1), 0.1);

        bool inside     = false;
        bool lastInside = false;

        for (int i = 0; i < steps; ++i) {
            float sampled = texture (agySampler, rayPos).r;
            if (sampled > minConsideredValue) {
                inside = true;
                if (lastInside == false) {
                }
                volume += sampled / steps;
                sampleCount++;
                if (volume > (hagymaSampledCount) * insideBrain) {
                    hagyma.w += (1.f - max (0, dot (GetNormalAt (rayPos), normalize (camera.viewDir)))) / maxHagymaSampleCount;
                    hagymaSampledCount++;
                
                    //hagymaSampledCount++;
                    //hagyma.w += (1.f - abs (dot (GetNormalAt (rayPos), normalize (camera.viewDir)))) / maxHagymaSampleCount;
                }
            } else {
                inside = false;
            }

            bool isHit = (volume > insideBrain);
            
            // hagyma
            isHit = (sampleCount == maxHagymaSampleCount);

            isHit = (hagymaSampledCount == maxHagymaSampleCount);

            float lastSampled = sampled;
            if (isHit) {

               // for (int j = 0; j < 32; ++j) {
               //     // binary search
               //     rayStep *= 0.5f;
               //
               //     if (isHit) {
               //         volume -= lastSampled;
               //         rayT -= rayStep;
               //     } else {
               //         rayT += rayStep;
               //     }
               //
               //     rayPos = camera.position + normRayDir * rayT;
               //
               //     float sampled = texture (agySampler, rayPos).r;
               //     lastSampled = sampled;
               //     volume += (sampled > minConsideredValue) ? sampled : 0.f;
               //
               //     isHit = (volume > insideBrain);
               // }
                
                gradient = vec4 (GetNormalAt (rayPos), 1.0f);
                break;
            }
            
            lastInside = inside;
            rayT += rayStep;
            rayPos = camera.position + normRayDir * rayT;
        }

        if (gradient.w > 0) {
            vec4 n = gradient * camera.viewMatrix;
            vec3 matcapTexture = texture (matcapSampler, (n.rg + 1) * 0.5).rgb;
            float angle = acos (dot (gradient.rgb, normalize (camera.viewDir)));

            presented = vec4 (vec3 (angle), gradient.w);
            presented = vec4 (vec3 (sampleCount/100.f), gradient.w);
            presented = vec4 (vec3 (angle), sampleCount/100.f);
            presented = vec4 (vec3 (sampleCount/100.f), 1.f);
            presented = hagyma;

            //presented = vec4 (matcapTexture.rgb, gradient.w);
            //presented = vec4 (gradient.rgb * 0.5 + 0.5, gradient.w);
        }
    }
    //presented = vec4 (texture (matcapSampler, uv).rgb, 1);

    //presented = vec4 (vec3 (bt.x), 1);

    //presented = vec4 (texture (agySampler, vec3 (uv.y, uv.x, fract(time.time*0.1f))).rrr, 1);
    
    copy[0] = vec4 (texture (agy2dSampler, uv).rgb, 1);
    copy[1] = result;
}