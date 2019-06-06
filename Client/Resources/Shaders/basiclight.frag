#version 440 core

in vec3 pass_fragPos;
in vec3 pass_normal;
in vec2 pass_uv;

struct Material
{
  sampler2D diffuse;
};

struct DirLight
{
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
};

struct PointLight
{
  vec3 position;
  
  vec3 ambient;
  vec3 diffuse;
  
  float constant;
  float linear;
  float quadratic;
};

// Lighting
uniform uint u_numdirlights = 0;
uniform uint u_numpointlights = 0;

uniform DirLight   u_dirlight;
uniform PointLight u_pointlight;
uniform Material   u_material;
uniform vec3       u_viewPos;
uniform float      u_transparency = 1.0;

// uniform int numBin = 30;

// Output
out vec4 out_color;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
  // Ambient
  vec3 ambient = light.ambient * texture(u_material.diffuse, pass_uv).rgb;

  // Diffuse
  vec3 lightDir = normalize(-light.direction);
  float diffAmt = max(dot(normal, lightDir), 0);
  vec3 diffuse = light.diffuse * diffAmt * texture(u_material.diffuse, pass_uv).rgb;

  return ambient + diffuse;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  // Ambient
  vec3 ambient = light.ambient * (texture(u_material.diffuse, pass_uv).rgb+ vec3(1.0));
  
  // Diffuse
  vec3 lightDir = normalize(light.position - fragPos);
  float diffAmt = max(dot(normal, lightDir), 0);
  vec3 diffuse = light.diffuse * diffAmt * (texture(u_material.diffuse, pass_uv).rgb+ vec3(1.0));
  
  // Attenuation
  float dist = length(light.position - fragPos);
  float attenuation = 1.0 / (light.constant +
                             (light.linear * dist) +
                             (light.quadratic * (dist * dist)));
  
  ambient  *= attenuation;
  diffuse  *= attenuation;
  
  return ambient + diffuse;
}

vec3 ToonShading(vec3 color) {
  vec3 origColor = texture(u_material.diffuse, pass_uv).rgb;
  float ratio = color.x / origColor.x;
  if (ratio > 0.8)
	return origColor * 0.9;
  return origColor * 0.8;
}

void main(void)
{
  vec3 normal = normalize(pass_normal);
  vec3 viewDir = normalize(u_viewPos - pass_fragPos);
  
  vec3 resultCol = vec3(0); // Final color

  // Calculate contribution of directional lights
  for (uint i = 0; i < u_numdirlights; i++)
  {
    resultCol += CalcDirLight(u_dirlight, normal, viewDir);
  }

  // Calculate contribution of point lights
  for (uint i = 0; i < u_numpointlights; i++)
  {
    resultCol += CalcPointLight(u_pointlight, normal, pass_fragPos, viewDir);
  }
  
  float alpha = 1.0f * u_transparency * texture(u_material.diffuse, pass_uv).a;

  if(alpha < 0.01){
	discard;
  }

  resultCol = ToonShading(resultCol);
  
  out_color = vec4(resultCol, alpha);

  // for wall transparent part
  if (texture(u_material.diffuse, pass_uv).a < 0.3)
	discard;
  
}
