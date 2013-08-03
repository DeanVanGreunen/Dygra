#include "level.h"
#include "tinyxml2.h"

#include "physfs.h"
#include "util.h"

using namespace tinyxml2;

Level::Level()
{
	shader = new sf::Shader();
}

bool replace(std::string& str, const std::string& from, const std::string& to)
{
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}


void Level::BuildShader(sf::Vector2f resolution)
{
	//Read template glsl into string
	std::string glsl = file_to_string( "shaders/template.frag" );

	std::stringstream code1;
	std::stringstream code2;

	std::stringstream code3;

	for (std::map<const char*, Object*>::iterator it=objects.begin(); it!=objects.end(); ++it)
	{
		(*it).second->objects = objects;

		code1 << (*it).second->GetDefinitionCode() << "\n";

		code2 << (*it).second->GetIntersectCode() << "\n";

		if((*it).second->type == "sphereaberration")
		{
			std::string aberration = file_to_string( "shaders/sphereaberrationinside.frag" );

			replaceAll(aberration, "{{NAME}}", (*it).second->name);

			code3 << aberration;
		}

		if((*it).second->type == "boxaberration")
		{
			std::string aberration = file_to_string( "shaders/boxaberrationinside.frag" );

			replaceAll(aberration, "{{NAME}}", (*it).second->name);

			code3 << aberration;
		}
	}


	replace(glsl, "{{OBJECTS}}", code1.str());


	replace(glsl, "{{INTERSECTCODE}}", code2.str());

	replace(glsl, "{{ABERRATION}}", code3.str());

	std::ofstream out;
	out.open("output.frag");
	out << glsl;
	out.close();


	shader->loadFromMemory(glsl, sf::Shader::Fragment);
	shader->setParameter("resolution", resolution);
}

void Level::SetCamera(sf::Vector3f pos, sf::Vector3f up, sf::Vector3f look)
{

	shader->setParameter("cameraPos", pos);
	shader->setParameter("cameraUp", up);
	shader->setParameter("cameraLook", look);
}

void Level::AddObject(Object* newobject)
{
	objects[newobject->name.c_str()] = newobject;
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

void Level::LoadXML(const char* filename)
{
	

	XMLDocument doc;
	doc.Parse( file_to_string(filename).c_str() );

	XMLElement* scene = doc.FirstChildElement("scene");

	for(const XMLElement* plane = scene->FirstChildElement("Plane"); plane; plane = plane->NextSiblingElement("Plane"))
	{
		std::string name = plane->Attribute("name");

		Plane *obj = new Plane(
			name,
			plane->FloatAttribute("x"),
			plane->FloatAttribute("y"),
			plane->FloatAttribute("z"),
			plane->FloatAttribute("c"),
			plane->FloatAttribute("reflectivity"),
			plane->BoolAttribute("uniform"),
			plane->BoolAttribute("recieve")
			);
		std::cout << obj->name << std::endl;
		
		this->AddObject(obj);
	}
	
	for(const XMLElement* sphere = scene->FirstChildElement("Sphere"); sphere; sphere = sphere->NextSiblingElement("Sphere"))
	{
		std::string name = sphere->Attribute("name");

		Sphere *obj = new Sphere(
			name,
			sphere->FloatAttribute("x"),
			sphere->FloatAttribute("y"),
			sphere->FloatAttribute("z"),
			sphere->FloatAttribute("r"),
			sphere->FloatAttribute("reflectivity"),
			sphere->BoolAttribute("uniform"),
			sphere->BoolAttribute("cast"),
			sphere->BoolAttribute("recieve")
			);
		std::cout << obj->name << std::endl;
		
		this->AddObject(obj);
	}

	for(const XMLElement* box = scene->FirstChildElement("Box"); box; box = box->NextSiblingElement("Box"))
	{
		std::string name = box->Attribute("name");

		std::vector<std::string> min = split( box->Attribute("min"), ',');
		std::vector<std::string> max = split( box->Attribute("max"), ',');

		Box *obj = new Box(
			name,
			atof(min[0].c_str()),
			atof(min[1].c_str()),
			atof(min[2].c_str()),
			atof(max[0].c_str()),
			atof(max[1].c_str()),
			atof(max[2].c_str()),
			box->BoolAttribute("uniform"),
			box->BoolAttribute("cast"),
			box->BoolAttribute("recieve")
			);

		std::cout << obj->name << std::endl;
		this->AddObject(obj);
	}

	for(const XMLElement* boxab = scene->FirstChildElement("BoxAberration"); boxab; boxab = boxab->NextSiblingElement("BoxAberration"))
	{
		std::string name = boxab->Attribute("name");

		std::vector<std::string> min = split( boxab->Attribute("min"), ',');
		std::vector<std::string> max = split( boxab->Attribute("max"), ',');

		std::vector<std::string> scale = split( boxab->Attribute("scale"), ',');

		Vector3f minf = Vector3f( atof(min[0].c_str()) , atof(min[1].c_str()), atof(min[2].c_str()));
		Vector3f maxf = Vector3f( atof(max[0].c_str()) , atof(max[1].c_str()), atof(max[2].c_str()));
		Vector3f scalef = Vector3f( atof(scale[0].c_str()) , atof(scale[1].c_str()), atof(scale[2].c_str()));

		BoxAberration *obj = new BoxAberration(name, minf, maxf, scalef, boxab->BoolAttribute("uniform"));

		std::cout << obj->name << std::endl;
		this->AddObject(obj);
	}

	for(const XMLElement* sphereab = scene->FirstChildElement("SphereAberration"); sphereab; sphereab = sphereab->NextSiblingElement("SphereAberration"))
	{
		std::string name = sphereab->Attribute("name");

		std::vector<std::string> scale = split( sphereab->Attribute("scale"), ',');
		//Vector3f scalef = Vector3f(  , , );

		SphereAberration *obj = new SphereAberration(
			name,
			sphereab->FloatAttribute("r"),
			sphereab->FloatAttribute("x"),
			sphereab->FloatAttribute("y"),
			sphereab->FloatAttribute("z"),

			atof(scale[0].c_str()),
			atof(scale[1].c_str()),
			atof(scale[2].c_str()),

			sphereab->BoolAttribute("uniform")
			);
		std::cout << obj->name << std::endl;
		
		this->AddObject(obj);
	}

	for(const XMLElement* sphereportal = scene->FirstChildElement("SpherePortal"); sphereportal; sphereportal = sphereportal->NextSiblingElement("SpherePortal"))
	{
		std::string name = sphereportal->Attribute("name");

		SpherePortal *obj = new SpherePortal(
			name,
			sphereportal->FloatAttribute("r"),

			sphereportal->FloatAttribute("x"),
			sphereportal->FloatAttribute("y"),
			sphereportal->FloatAttribute("z"),

			sphereportal->FloatAttribute("x2"),
			sphereportal->FloatAttribute("y2"),
			sphereportal->FloatAttribute("z2"),
			
			sphereportal->BoolAttribute("uniform")
			);
		std::cout << obj->name << std::endl;
		
		this->AddObject(obj);
	}

	
}