/*	
	Author: Richard McKenna
			Stony Brook University
			Computer Science Department

	SpriteManager.cpp

	See SpriteManager.h for a class description.
*/

#pragma once
#include "sssf_VS\stdafx.h"
#include "sssf\gsm\ai\Bot.h"
#include "sssf\gsm\physics\PhysicalProperties.h"
#include "sssf\graphics\GameGraphics.h"
#include "sssf\gsm\sprite\AnimatedSprite.h"
#include "sssf\gsm\sprite\AnimatedSpriteType.h"
#include "sssf\gsm\sprite\SpriteManager.h"
#include "sssf\gsm\state\GameStateManager.h"
#include <sssf/gsm/ai/bots/RandomJumpingBot.h>

/*
	addSpriteToRenderList - This method checks to see if the sprite
	parameter is inside the viewport. If it is, a RenderItem is generated
	for that sprite and it is added to the render list.
*/
void SpriteManager::addSpriteToRenderList(AnimatedSprite *sprite,
										  RenderList *renderList,
										  Viewport *viewport)
{
	// GET THE SPRITE TYPE INFO FOR THIS SPRITE
	AnimatedSpriteType *spriteType = sprite->getSpriteType();
	PhysicalProperties *pp = sprite->getPhysicalProperties();

	// IS THE SPRITE VIEWABLE?
	if (viewport->areWorldCoordinatesInViewport(	
									pp->getX(),
									pp->getY(),
									spriteType->getTextureWidth(),
									spriteType->getTextureHeight()))
	{
		// SINCE IT'S VIEWABLE, ADD IT TO THE RENDER LIST
		RenderItem itemToAdd;
		itemToAdd.id = sprite->getFrameIndex();
		renderList->addRenderItem(	sprite->getCurrentImageID(),
									pp->round(pp->getX()-viewport->getViewportX()),
									pp->round(pp->getY()-viewport->getViewportY()),
									pp->round(pp->getZ()),
									sprite->getAlpha(),
									spriteType->getTextureWidth(),
									spriteType->getTextureHeight());
	}
}

/*
	addSpriteItemsToRenderList - This method goes through all of the sprites,
	including the player sprite, and adds the visible ones to the render list.
	This method should be called each frame.
*/
void SpriteManager::addSpriteItemsToRenderList(	Game *game)
{
	GameStateManager *gsm = game->getGSM();
	GameGUI *gui = game->getGUI();
	if (gsm->isWorldRenderable())
	{
		GameGraphics *graphics = game->getGraphics();
		RenderList *renderList = graphics->getWorldRenderList();
		Viewport *viewport = gui->getViewport();

		// ADD THE PLAYER SPRITE
		player.checkVisible();
		if (player.isVisible()) {
			addSpriteToRenderList(&player, renderList, viewport);
		}

		// NOW ADD THE REST OF THE SPRITES
		list<Bot*>::iterator botIterator;
		botIterator = bots.begin();
		while (botIterator != bots.end())
		{			
			Bot *bot = (*botIterator);
			addSpriteToRenderList(bot, renderList, viewport);
			botIterator++;
		}

		list<Bot*>::iterator botDeathIterator;
		botDeathIterator = dyingBots.begin();
		while (botDeathIterator != dyingBots.end())
		{
			Bot *bot = (*botDeathIterator);
			addSpriteToRenderList(bot, renderList, viewport);
			botDeathIterator++;
		}
	}
}

/*
	addSprite - This method is for adding a new sprite to 
	this sprite manager. Once a sprite is added it can be 
	scheduled for rendering.
*/
void SpriteManager::addBot(Bot *botToAdd)
{
	bots.push_back(botToAdd);
}

/*
	addSpriteType - This method is for adding a new sprite
	type. Note that one sprite type can have many sprites. For
	example, we may say that there may be a "Bunny" type of
	sprite, and specify properties for that type. Then there might
	be 100 different Bunnies each with their own properties, but that
	share many things in common according to what is defined in
	the shared sprite type object.
*/
unsigned int SpriteManager::addSpriteType(AnimatedSpriteType *spriteTypeToAdd)
{
	spriteTypes.push_back(spriteTypeToAdd);
	return spriteTypes.size()-1;
}

/*
	clearSprites - This empties all of the sprites and sprite types.
*/
void SpriteManager::clearSprites()
{
	spriteTypes.clear();
	bots.clear();
}

/*
	getSpriteType - This gets the sprite type object that corresponds
	to the index argument.
*/
AnimatedSpriteType* SpriteManager::getSpriteType(unsigned int typeIndex)
{
	if (typeIndex < spriteTypes.size())
		return spriteTypes.at(typeIndex);
	else
		return NULL;
}

/*
	unloadSprites - This method removes all artwork from memory that
	has been allocated for game sprites.
*/
void SpriteManager::unloadSprites()
{
	bots.clear();
	// @TODO - WE'LL DO THIS LATER WHEN WE LEARN MORE ABOUT MEMORY MANAGEMENT
}

Bot* SpriteManager::removeBot(Bot *botToRemove)
{
	return botToRemove;
	// @TODO - WE'LL DO THIS LATER WHEN WE LEARN MORE ABOUT MEMORY MANAGEMENT
}

/*
	update - This method should be called once per frame. It
	goes through all of the sprites, including the player, and calls their
	update method such that they may update themselves.
*/
void SpriteManager::update(Game *game)
{
	// UPDATE THE PLAYER SPRITE
	player.checkDead();
	player.updateSprite();

	// NOW UPDATE THE REST OF THE SPRITES
	list<Bot*>::iterator botIterator;
	botIterator = bots.begin();
	while (botIterator != bots.end())
	{
		Bot *bot = (*botIterator);
		bot->think(game);
		bot->updateSprite();
		botIterator++;
	}

	list<Bot*>::iterator botDeathIterator;
	botDeathIterator = dyingBots.begin();
	int deathCount = 0;
	while (botDeathIterator != dyingBots.end())
	{
		Bot *bot = (*botDeathIterator);
		bot->updateSprite();
		bot->kill();
		if (bot->getDying() == 18) {
			deathCount++;
		}
		botDeathIterator++;
	}
	while (deathCount>0) {
		dyingBots.pop_front();
		deathCount--;
	}
}

void SpriteManager::removeBot() {
	bots.pop_back();
}

void SpriteManager::removeBotFromList(Bot* bot) {
	bots.remove(bot);
}

void SpriteManager::checkCollision(Physics* physics, int playerIndex) {
	std::list<Bot*> botList = botTree.getBotsInNode(playerIndex);
	for (Bot* bot : botList) {
		physics->addBotCollision(&player, bot);
	}
}

void SpriteManager::killBot(Bot* bot) {
	removeBotFromList(bot);
	dyingBots.push_back(bot);
}

void SpriteManager::generateBots(Game* game) {
	if (disf(gen2)<0.02) {
		if (disf(gen2)<0.2) {
			AnimatedSpriteType *botHealthSprite = getSpriteType(0);
			int generatedRandom = dis(gen2);
			if (generatedRandom == 1) {
				makeRandomJumpingBot(game, botHealthSprite, 80 , 1350 );
			}
			else if (generatedRandom == 2) {
				makeRandomJumpingBot(game, botHealthSprite, 2800, 100);
			}
			else {
				makeRandomJumpingBot(game, botHealthSprite, 3072 , 1850);
			}
		}
		else {
			AnimatedSpriteType *botHealthSprite = getSpriteType(1);
			int generatedRandom = dis(gen2);
			if (generatedRandom == 1) {
				makeRandomJumpingBot(game, botHealthSprite, 80, 1350);
			}
			else if (generatedRandom == 2) {
				makeRandomJumpingBot(game, botHealthSprite, 2800, 100);
			}
			else {
				makeRandomJumpingBot(game, botHealthSprite, 3072, 1850);
			}
		}
	}
}

void SpriteManager::makeRandomJumpingBot(Game *game, AnimatedSpriteType *randomJumpingBotType, float initX, float initY)
{
	SpriteManager *spriteManager = game->getGSM()->getSpriteManager();
	Physics *physics = game->getGSM()->getPhysics();
	RandomJumpingBot *bot = new RandomJumpingBot(physics, 30, 120, 40);
	physics->addCollidableObject(bot);
	PhysicalProperties *pp = bot->getPhysicalProperties();
	pp->setPosition(initX, initY);
	bot->setSpriteType(randomJumpingBotType);
	bot->setCurrentState(L"JUMPING");
	bot->setAlpha(255);
	spriteManager->addBot(bot);
	bot->affixTightAABBBoundingVolume();
}