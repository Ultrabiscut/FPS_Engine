#pragma once

class GameMaterial : public Material
{
public:
	GameMaterial();
	GameMaterial(char *name, char *path = "./");

	virtual ~GameMaterial();

	//Sound *GetStepSound();
	//LinkedList< Sound > *m_stepSounds;
};

