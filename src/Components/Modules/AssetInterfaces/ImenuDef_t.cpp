#include "STDInclude.hpp"

namespace Assets
{
	void ImenuDef_t::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::menuDef_t *asset = header.menu;

		if (asset->window.background)
		{
			builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->window.background);
		}
		
		// mark items
		for (int i = 0; i < asset->itemCount; i++)
		{
			if (asset->items[i]->window.background)
			{
				builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->items[i]->window.background);
			}

			if (asset->items[i]->focusSound)
			{
				builder->loadAsset(Game::XAssetType::ASSET_TYPE_SOUND, asset->items[i]->focusSound);
			}
		}
	}

	template <typename T> 
	void save_windowDef_t(Game::windowDef_t* asset, T* dest, Components::ZoneBuilder::Zone* builder)
	{
		Utils::Stream* buffer = builder->getBuffer();

		if (asset->name)
		{
			buffer->saveString(asset->name);
			Utils::Stream::ClearPointer(&dest->window.name);
		}

		if (asset->group)
		{
			buffer->saveString(asset->group);
			Utils::Stream::ClearPointer(&dest->window.group);
		}

		if (asset->background)
		{
			dest->window.background = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->background).material;
		}
	}

	void save_Statement_s(Game::Statement_s* asset, Components::ZoneBuilder::Zone* builder);
	void save_ExpressionSupportingData(Game::ExpressionSupportingData* asset, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::ExpressionSupportingData, 24);
		Utils::Stream* buffer = builder->getBuffer();

		buffer->align(Utils::Stream::ALIGN_4);

		Game::ExpressionSupportingData *dest = buffer->dest<Game::ExpressionSupportingData>();
		buffer->save(asset);

		if (asset->uifunctions.functions)
		{
			buffer->align(Utils::Stream::ALIGN_4);

			Game::Statement_s **destStatement = buffer->dest<Game::Statement_s*>();
			buffer->saveArray(asset->uifunctions.functions, asset->uifunctions.totalFunctions);

			for (int i = 0; i < asset->uifunctions.totalFunctions; i++)
			{
				if (asset->uifunctions.functions[i])
				{
					Utils::Stream::ClearPointer(&destStatement[i]);

					buffer->align(Utils::Stream::ALIGN_4);
					save_Statement_s(asset->uifunctions.functions[i], builder);
				}
			}

			Utils::Stream::ClearPointer(&dest->uifunctions.functions);
		}

		if (asset->staticDvarList.staticDvars)
		{
			buffer->align(Utils::Stream::ALIGN_4);

			Game::StaticDvar **destStaticDvars = buffer->dest<Game::StaticDvar*>();
			buffer->saveArray(asset->staticDvarList.staticDvars, asset->staticDvarList.numStaticDvars);

			for (int i = 0; i < asset->staticDvarList.numStaticDvars; i++)
			{
				if (asset->staticDvarList.staticDvars[i])
				{
					Utils::Stream::ClearPointer(&destStaticDvars[i]);

					buffer->align(Utils::Stream::ALIGN_4);
					Game::StaticDvar *destStaticDvar = buffer->dest<Game::StaticDvar>();
					buffer->save(asset->staticDvarList.staticDvars[i]);

					if (asset->staticDvarList.staticDvars[i]->dvarName)
					{
						buffer->saveString(asset->staticDvarList.staticDvars[i]->dvarName);
						Utils::Stream::ClearPointer(&destStaticDvar->dvarName);
					}
				}
			}

			Utils::Stream::ClearPointer(&dest->staticDvarList.staticDvars);
		}

		if (asset->uiStrings.strings)
		{
			buffer->align(Utils::Stream::ALIGN_4);

			const char **destuiStrings = buffer->dest<const char*>();
			buffer->saveArray(asset->uiStrings.strings, asset->uiStrings.totalStrings);

			for (int i = 0; i < asset->uiStrings.totalStrings; i++)
			{
				if (asset->uiStrings.strings[i])
				{
					buffer->saveString(asset->uiStrings.strings[i]);
					Utils::Stream::ClearPointer(&destuiStrings[i]);
				}
			}
		}
	}

	void save_Statement_s(Game::Statement_s* asset, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::Statement_s, 24);
		Utils::Stream* buffer = builder->getBuffer();

		// Write header data
		Game::Statement_s *dest = buffer->dest<Game::Statement_s>();
		buffer->save(asset);

		// Write statement entries
		if (asset->entries)
		{
			buffer->align(Utils::Stream::ALIGN_4);

			// Write entries
			Game::expressionEntry *destEntries = buffer->dest<Game::expressionEntry>();
			buffer->save(asset->entries, sizeof(Game::expressionEntry), asset->numEntries);

			// Loop through entries
			for (int i = 0; i < asset->numEntries; i++)
			{
				if (asset->entries[i].type)
				{
					switch (asset->entries[i].data.operand.dataType)
					{
						// Those types do not require additional data
					case 0:
					case 1:
						break;

						// Expression string
					case 2:
						if (asset->entries[i].data.operand.internals.stringVal.string)
						{
							buffer->saveString(asset->entries[i].data.operand.internals.stringVal.string);
							Utils::Stream::ClearPointer(&destEntries[i].data.operand.internals.stringVal.string);
						}
						break;

						// Function
					case 3:
						if (asset->entries[i].data.operand.internals.function)
						{
							save_Statement_s(asset->entries[i].data.operand.internals.function, builder);
							Utils::Stream::ClearPointer(&destEntries[i].data.operand.internals.function);
						}
						break;
					}
				}
			}
		}
		if (asset->supportingData)
		{
			save_ExpressionSupportingData(asset->supportingData, builder);
			Utils::Stream::ClearPointer(&dest->supportingData);
		}
	}

	void save_MenuEventHandlerSet(Game::MenuEventHandlerSet* asset, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::MenuEventHandlerSet, 8);
		Utils::Stream* buffer = builder->getBuffer();

		// Write header data
		Game::MenuEventHandlerSet *destset = buffer->dest<Game::MenuEventHandlerSet>();
		buffer->save(asset);

		// Event handlers
		if (asset->eventHandlers)
		{
			buffer->align(Utils::Stream::ALIGN_4);

			// Write pointers to zone
			buffer->save(asset->eventHandlers, sizeof(Game::MenuEventHandler*), asset->eventHandlerCount);

			// Loop through eventHandlers
			for (int i = 0; i < asset->eventHandlerCount; i++)
			{
				if (asset->eventHandlers[i])
				{
					buffer->align(Utils::Stream::ALIGN_4);

					// Write menu event handler
					Game::MenuEventHandler *dest = buffer->dest<Game::MenuEventHandler>();
					buffer->save(asset->eventHandlers[i]);

					// Write additional data based on type
					switch (asset->eventHandlers[i]->eventType)
					{
						// unconditional scripts
					case 0:
						if (asset->eventHandlers[i]->eventData.unconditionalScript)
						{
							buffer->saveString(asset->eventHandlers[i]->eventData.unconditionalScript);
							Utils::Stream::ClearPointer(&dest->eventData.unconditionalScript);
						}
						break;

						// ConditionalScript
					case 1:
						if (asset->eventHandlers[i]->eventData.conditionalScript)
						{
							buffer->align(Utils::Stream::ALIGN_4);
							Game::ConditionalScript *destConditionalScript = buffer->dest<Game::ConditionalScript>();
							buffer->save(asset->eventHandlers[i]->eventData.conditionalScript);

							// eventExpression
							if (asset->eventHandlers[i]->eventData.conditionalScript->eventExpression)
							{
								buffer->align(Utils::Stream::ALIGN_4);
								save_Statement_s(asset->eventHandlers[i]->eventData.conditionalScript->eventExpression, builder);
								Utils::Stream::ClearPointer(&destConditionalScript->eventExpression);
							}

							// eventHandlerSet
							if (asset->eventHandlers[i]->eventData.conditionalScript->eventHandlerSet)
							{
								buffer->align(Utils::Stream::ALIGN_4);
								save_MenuEventHandlerSet(asset->eventHandlers[i]->eventData.conditionalScript->eventHandlerSet, builder);
								Utils::Stream::ClearPointer(&destConditionalScript->eventHandlerSet);
							}

							Utils::Stream::ClearPointer(&dest->eventData.conditionalScript);
						}
						break;

						// elseScript
					case 2:
						if (asset->eventHandlers[i]->eventData.elseScript)
						{
							buffer->align(Utils::Stream::ALIGN_4);
							save_MenuEventHandlerSet(asset->eventHandlers[i]->eventData.elseScript, builder);
							Utils::Stream::ClearPointer(&dest->eventData.elseScript);
						}
						break;

						// localVarData expressions
					case 3:
					case 4:
					case 5:
					case 6:
						if (asset->eventHandlers[i]->eventData.setLocalVarData)
						{
							// header data
							Game::SetLocalVarData *destLocalVarData = buffer->dest<Game::SetLocalVarData>();
							buffer->save(asset->eventHandlers[i]->eventData.setLocalVarData);

							buffer->align(Utils::Stream::ALIGN_4);

							// localVarName
							if (asset->eventHandlers[i]->eventData.setLocalVarData->localVarName)
							{
								buffer->saveString(asset->eventHandlers[i]->eventData.setLocalVarData->localVarName);
								Utils::Stream::ClearPointer(&destLocalVarData->localVarName);
							}

							// statement
							if (asset->eventHandlers[i]->eventData.setLocalVarData->expression)
							{
								buffer->align(Utils::Stream::ALIGN_4);
								save_Statement_s(asset->eventHandlers[i]->eventData.setLocalVarData->expression, builder);
								Utils::Stream::ClearPointer(&destLocalVarData->expression);
							}

							Utils::Stream::ClearPointer(&dest->eventData.setLocalVarData);
						}
						break;
					}
				}
			}

			Utils::Stream::ClearPointer(&destset->eventHandlers);
		}
	}

	void save_ItemKeyHandler(Game::ItemKeyHandler* asset, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::ItemKeyHandler, 12);
		Utils::Stream* buffer = builder->getBuffer();

		while (asset)
		{
			// align every indice
			buffer->align(Utils::Stream::ALIGN_4);

			// Write header
			Game::ItemKeyHandler* dest = buffer->dest<Game::ItemKeyHandler>();
			buffer->save(asset);

			// MenuEventHandlerSet
			if (asset->action)
			{
				buffer->align(Utils::Stream::ALIGN_4);
				save_MenuEventHandlerSet(asset->action, builder);
				Utils::Stream::ClearPointer(&dest->action);
			}

			// Next key handler
			asset = asset->next;
		}
	}

#define EVENTHANDLERSET(__indice) \
		if (asset->__indice) \
		{ \
			buffer->align(Utils::Stream::ALIGN_4); \
			save_MenuEventHandlerSet(asset->__indice, builder); \
			Utils::Stream::ClearPointer(&dest->__indice); \
		}

#define STATEMENT(__indice) \
		if (asset->__indice) \
		{ \
			buffer->align(Utils::Stream::ALIGN_4); \
			save_Statement_s(asset->__indice, builder); \
			Utils::Stream::ClearPointer(&dest->__indice); \
		}

	void save_itemDef_t(Game::itemDef_t *asset, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::itemDef_t, 380);

		Utils::Stream* buffer = builder->getBuffer();
		Game::itemDef_t* dest = buffer->dest<Game::itemDef_t>();

		buffer->save(asset);

		// window data
		save_windowDef_t<Game::itemDef_t>(&asset->window, dest, builder);

		// text
		if (asset->text)
		{
			buffer->saveString(asset->text);
			Utils::Stream::ClearPointer(&dest->text);
		}

		// MenuEventHandlerSets
		EVENTHANDLERSET(mouseEnterText);
		EVENTHANDLERSET(mouseExitText);
		EVENTHANDLERSET(mouseEnter);
		EVENTHANDLERSET(mouseExit);
		EVENTHANDLERSET(action);
		EVENTHANDLERSET(accept);
		EVENTHANDLERSET(onFocus);
		EVENTHANDLERSET(leaveFocus);

		// Dvar strings
		if (asset->dvar)
		{
			buffer->saveString(asset->dvar);
			Utils::Stream::ClearPointer(&dest->dvar);
		}
		if (asset->dvarTest)
		{
			buffer->saveString(asset->dvarTest);
			Utils::Stream::ClearPointer(&dest->dvarTest);
		}

		// ItemKeyHandler
		if (asset->onKey)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			save_ItemKeyHandler(asset->onKey, builder);
			Utils::Stream::ClearPointer(&dest->onKey);
		}

		// Dvar strings
		if (asset->enableDvar)
		{
			buffer->saveString(asset->enableDvar);
			Utils::Stream::ClearPointer(&dest->enableDvar);
		}
		if (asset->localVar)
		{
			buffer->saveString(asset->localVar);
			Utils::Stream::ClearPointer(&dest->localVar);
		}

		// Focus sound
		if (asset->focusSound)
		{
			dest->focusSound = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_SOUND, asset->focusSound).sound;
		}

		// itemDefData (fix me)
		dest->typeData.data = nullptr;

		// floatExpressions (fix me)
		dest->floatExpressions = nullptr;
		dest->floatExpressionCount = 0;

		// Statements
		STATEMENT(visibleExp);
		STATEMENT(disabledExp);
		STATEMENT(textExp);
		STATEMENT(materialExp);
	}

	void ImenuDef_t::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::menuDef_t, 400);

		Utils::Stream* buffer = builder->getBuffer();
		Game::menuDef_t* asset = header.menu;
		Game::menuDef_t* dest = buffer->dest<Game::menuDef_t>();

		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		// ExpressionSupportingData
		if (asset->expressionData)
		{
			save_ExpressionSupportingData(asset->expressionData, builder);
			Utils::Stream::ClearPointer(&dest->expressionData);
		}

		// Window data
		save_windowDef_t<Game::menuDef_t>(&asset->window, dest, builder);

		// Font
		if (asset->font)
		{
			buffer->saveString(asset->font);
			Utils::Stream::ClearPointer(&dest->font);
		}

		// MenuEventHandlerSets
		EVENTHANDLERSET(onOpen);
		EVENTHANDLERSET(onRequestClose);
		EVENTHANDLERSET(onClose);
		EVENTHANDLERSET(onEsc);

		// ItemKeyHandler
		if (asset->onKey)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			save_ItemKeyHandler(asset->onKey, builder);
			Utils::Stream::ClearPointer(&dest->onKey);
		}

		// Statement
		STATEMENT(visibleExp);

		// Strings
		if (asset->allowedBinding)
		{
			buffer->saveString(asset->allowedBinding);
			Utils::Stream::ClearPointer(&dest->allowedBinding);
		}
		if (asset->soundLoop)
		{
			buffer->saveString(asset->soundLoop);
			Utils::Stream::ClearPointer(&dest->soundLoop);
		}

		// Statements
		STATEMENT(rectXExp);
		STATEMENT(rectYExp);
		STATEMENT(rectHExp);
		STATEMENT(rectWExp);
		STATEMENT(openSoundExp);
		STATEMENT(closeSoundExp);

		// Items
		if (asset->items)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->items, asset->itemCount);

			for (int i = 0; i < asset->itemCount; i++)
			{
				if (asset->items[i])
				{
					buffer->align(Utils::Stream::ALIGN_4);
					save_itemDef_t(asset->items[i], builder);
				}
			}
		}

		buffer->popBlock();
	}
}
