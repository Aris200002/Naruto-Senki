#include "Effect.h"
#include "CharacterBase.h"
#include "GameLayer.h"

bool Effect::init(const string &name, CharacterBase *attacker)
{
	RETURN_FALSE_IF(!Sprite::init());

	at = attacker;

	if (name == "n_hit" ||
		name == "b_hit")
	{
		auto effectAction = createEffectAnimation("red_damge_", 3, 14, false);
		runAction(effectAction);

		setAnchorPoint(Vec2(0.5f, 0));
		setScale(0.6f);
		float randomY = rand() % 16;
		float randomX = rand() % 4;
		setPosition(Vec2(at->getPositionX() + randomX, at->getPositionY() + randomY));
	}
	else if (name == "l_hit" ||
			 name == "f_hit" ||
			 name == "f2_hit" ||
			 name == "c_hit" ||
			 name == "o_hit" ||
			 name == "o2_hit" ||
			 name == "ob_hit" ||
			 name == "ab_hit" ||
			 name == "ac_hit" ||
			 name == "ct_hit" ||
			 name == "ts_hit" ||
			 name == "s_hit")
	{
		auto effectAction = createEffectAnimation("red_damge_", 3, 14, false);
		runAction(effectAction);

		setScale(0.8f);
		setAnchorPoint(Vec2(0.5f, 0));
		setPosition(Vec2(at->getPositionX(), at->getPositionY()));
	}
	else if (name == "bl_hit" ||
			 name == "bc_hit" ||
			 name == "bf_hit" ||
			 name == "sl_hit")
	{
		// setAnchorPoint(Vec2(0.5f,0));
		auto effectAction = createEffectAnimation("blue_damge_", 3, 14, false);
		runAction(effectAction);

		setScale(0.6f);
		int randomInt = rand() % 30 + 10;
		setRotation(randomInt);
		setPosition(Vec2(at->getPositionX(), at->getPositionY() + at->getContentSize().height / 2));
	}
	else if (name == "a_hit")
	{
		auto effectAction = createEffectAnimation("bottom_damage_", 5, 20, false);
		runAction(effectAction);

		setAnchorPoint(Vec2(0.5f, 0));
		setPosition(Vec2(at->getPositionX() + 16, at->getPositionY()));
	}
	else if (name == "Kagura")
	{
		auto effectAction = createEffectAnimation("Kagura_", 8, 10, false);
		runAction(effectAction);
	}
	else if (name == "Hupo")
	{
		auto effectAction = createEffectAnimation("Hupo_", 6, 10, false);
		runAction(effectAction);
	}
	else if (name == "hp_restore" ||
			 name == "speedUp")
	{
		auto effectAction = createEffectAnimation(name + "_", 4, 5, false);
		auto call = CallFunc::create(std::bind(&CharacterBase::disableEffect, at));
		auto seq = newSequence(effectAction, call);
		runAction(seq);
	}
	else if (name == "smk")
	{
		auto effectAction = createEffectAnimation("smk_", 4, 10, false);
		runAction(effectAction);
		setAnchorPoint(Vec2(0.5f, 0));
		setPosition(at->getPosition());

		if (UserDefault::sharedUserDefault()->getBoolForKey("isVoice"))
			SimpleAudioEngine::sharedEngine()->playEffect("Audio/Effect/poof.ogg");
	}
	else if (name == "tishen")
	{
		auto effectAction = createEffectAnimation("tishen_", 6, 10, false);
		runAction(effectAction);
		setAnchorPoint(Vec2(0.5f, 0));
		setPosition(at->getPosition());
	}
	else if (name == "stun")
	{
		auto effectAction = createEffectAnimation("stun_", 4, 5, false);
		runAction(effectAction);
		setAnchorPoint(Vec2(0.5f, 0));
		setPosition(Vec2(at->getPositionX(), at->getPositionY() + at->getContentSize().height - 2));
	}
	else if (name == "DarkFlame")
	{
		auto effectAction = createEffectAnimation("DarkFlame_Effect_", 4, 10, true);
		runAction(effectAction);

		setAnchorPoint(Vec2(0.5f, 0));
		setPosition(Vec2(at->getPositionX(), at->getPositionY()));

		auto delay = DelayTime::create(2.8f);
		auto call = CallFunc::create(std::bind(&Effect::removeEffect, this));
		auto seq = newSequence(delay, call);
		runAction(seq);
	}
	else if (name == "Bagua")
	{
		auto effectAction = createEffectAnimation("Bagua_", 23, 10, false);
		runAction(effectAction);
		setAnchorPoint(Vec2(0.5f, 0));
		setPosition(Vec2(at->getPositionX() + 2, at->getPositionY() - 52));
	}
	else if (name == "Kujiyose")
	{
		initWithSpriteFrameName(name.c_str());
		setAnchorPoint(Vec2(0.5f, 0));
		setPosition(Vec2(at->getPositionX(), at->getPositionY() - getContentSize().height / 2));
		auto delay = DelayTime::create(0.3f);
		auto call = CallFunc::create(std::bind(&Effect::removeEffect, this));
		auto seq = newSequence(delay, call);
		runAction(seq);
	}
	else if (name == "kazi")
	{
		initWithSpriteFrameName(name.c_str());
		setAnchorPoint(Vec2(0.5f, 0));
		setPosition(Vec2(at->getPositionX() + (at->_isFlipped ? -32 : 32),
						 at->getPositionY() + at->getContentSize().height / 2));
		auto su = ScaleBy::create(0.1f, 1.2f);
		auto call = CallFunc::create(std::bind(&Effect::removeEffect, this));
		auto seq = newSequence(su, su->reverse(), call);
		runAction(seq);
	}
	else if (name == "sharingan" ||
			 name == "sharingan2" ||
			 name == "sharingan3" ||
			 name == "sharingan4")
	{
		initWithSpriteFrameName(name.c_str());
		setPosition(Vec2(at->getPositionX() + (at->_isFlipped ? -32 : 32),
						 at->getPositionY() + at->getContentSize().height));
		auto rt = RotateBy::create(0.3f, 180, 180);
		auto su = ScaleBy::create(0.2f, 1.6f);
		auto call = CallFunc::create(std::bind(&Effect::removeEffect, this));
		auto seq = newSequence(rt, su, call);
		runAction(seq);
	}
	else if (name == "Hiraishin" ||
			 name == "Hiraishin2")
	{
		auto effectAction = createEffectAnimation(name + "_Effect_", 3, 10, false);
		runAction(effectAction);
		setAnchorPoint(Vec2(0.5f, 0));
		setPosition(at->getPosition());
	}
	else if (name == "hBuff")
	{
		auto effectAction = createEffectAnimation(name + "_Effect_", 3, 5, true);
		runAction(effectAction);
		setAnchorPoint(Vec2(0.5f, 0));
	}
	else if (name == "sBuff")
	{
		auto effectAction = createEffectAnimation(name + "_Effect_", 10, 10, true);
		runAction(effectAction);
		setAnchorPoint(Vec2(0.5f, 0));
	}
	else if (name == "hsBuff" ||
			 name == "tBuff")
	{
		auto effectAction = createEffectAnimation(name + "_Effect_", 12, 10, true);
		runAction(effectAction);
		setAnchorPoint(Vec2(0.5f, 0));
	}
	else if (name == "dcBuff")
	{
		auto effectAction = createEffectAnimation(name + "_Effect_", 10, 10, true);
		runAction(effectAction);
		setAnchorPoint(Vec2(0.5f, 0));
	}
	else if (name == "jdBuff" ||
			 name == "bmBuff")
	{
		auto effectAction = createEffectAnimation(name + "_Effect_", 8, 10, true);
		runAction(effectAction);
		setAnchorPoint(Vec2(0.5f, 0));
	}
	else if (name == "dhBuff")
	{
		for (int i = 0; i < 3; i++)
		{
			auto ef = Sprite::createWithSpriteFrameName("FireEffect_01");
			ef->setAnchorPoint(Vec2(0.5f, 0));
			if (i == 0)
			{
				ef->setPosition(Vec2(10, 0));
			}
			else if (i == 1)
			{
				ef->setPosition(Vec2(-10, 18));
			}
			else
			{
				ef->setPosition(Vec2(10, 34));
			}
			auto effectAction = createEffectAnimation("FireEffect_", 4, 10, true);
			ef->runAction(effectAction);
			addChild(ef);
		}
	}
	else
	{
		CCLOG("Create effect failed by name %s", name.c_str());
		return false;
	}

	return true;
}

FiniteTimeAction *Effect::createEffectAnimation(const string &file, size_t frameCount, uint8_t fps, bool isLoop)
{
	Vector<SpriteFrame *> spriteFrames(frameCount);

	for (size_t i = 1; i < frameCount + 1; i++)
	{
		auto frame = getSpriteFrame("{}{:02d}", file, i);
		spriteFrames.pushBack(frame);

		// NOTE: For cocos2d-x 2.x using SpriteBatchNode,
		// the sprite must be initialized before SpriteBatchNode::addChild
		// Otherwise you will get an empty texture error
		if (i == 1)
			initWithSpriteFrame(frame);
	}

	auto animation = Animation::createWithSpriteFrames(spriteFrames, 1.0 / fps);
	auto animAction = Animate::create(animation);

	if (isLoop)
	{
		return RepeatForever::create(animAction);
	}
	else
	{
		auto call = CallFunc::create(std::bind(&Effect::removeEffect, this));
		return newSequence(animAction, call);
	}
}

FiniteTimeAction *Effect::createFontAnimation()
{
	auto delay = DelayTime::create(0.3f);
	auto call = CallFunc::create(std::bind(&Effect::removeFontEffect, this));
	auto seq = newSequence(delay, call);
	return seq;
}

void Effect::removeEffect()
{
	if (at->damageEffectCount > 0)
	{
		at->damageEffectCount--;
	}
	stopAllActions();
	removeFromParent();
}

void Effect::removeFontEffect()
{
	stopAllActions();
	removeFromParent();
}

Effect *Effect::create(const string &szImage, CharacterBase *attacker)
{
	Effect *ef = new Effect();
	if (ef && ef->init(szImage, attacker))
	{
		ef->autorelease();
		return ef;
	}
	else
	{
		delete ef;
		return nullptr;
	}
}
