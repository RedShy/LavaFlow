#version 330 core

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

in vec3 FragPos;
in vec3 Normal;
in vec3 RedValue;
in vec2 TexCoord;

out vec4 color;

uniform vec3 viewPos;
uniform Light light;
uniform sampler2D texture1;

void main()
{
    vec3 aColor;
    if (RedValue == 0.0f)
    {
        aColor = vec3(texture(texture1, TexCoord).rgb) * vec3(1.0f);
    }
    else
    {
        aColor = vec3(1.0f, RedValue.x, 0.0f);
    }

    // Ambient
    vec3 ambient = light.ambient * aColor;

   // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * aColor;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = light.specular * spec * aColor;

    // Attenuation
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    //ambient  *= attenuation;
    //diffuse  *= attenuation;
    //specular *= attenuation;

    color = vec4(ambient + diffuse + specular, 1.0f);
}


