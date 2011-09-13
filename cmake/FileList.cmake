set(BASE_SRC 
src/configuration.cpp
src/globals.cpp
src/main-menu.cpp
src/nameplacer.cpp)

set(GAME_SRC
src/paintown-engine/game/world.cpp
src/paintown-engine/game/game.cpp
src/paintown-engine/game/move-list.cpp
src/paintown-engine/game/select_player.cpp
src/util/console.cpp
src/paintown-engine/game/adventure_world.cpp
src/paintown-engine/game/mod.cpp
src/paintown-engine/game/options.cpp
src/util/input/input.cpp
src/util/input/text-input.cpp
src/util/input/input-manager.cpp
src/util/input/input-source.cpp)

set(SCRIPT_SRC
src/paintown-engine/script/script.cpp
src/paintown-engine/script/modules/python.cpp)

set(LEVEL_SRC
src/paintown-engine/level/block.cpp
src/paintown-engine/level/blockobject.cpp
src/paintown-engine/level/cacher.cpp
src/paintown-engine/level/scene.cpp
src/paintown-engine/level/random-scene.cpp
src/paintown-engine/level/utils.cpp)

set(NETWORK_SRC
src/util/network/network.cpp
src/util/network/network-system.cpp
src/paintown-engine/network/cacher.cpp
src/paintown-engine/network/client.cpp
src/paintown-engine/network/chat_client.cpp
src/paintown-engine/network/chat_server.cpp
src/paintown-engine/network/chat-widget.cpp
src/paintown-engine/network/network_world.cpp
src/paintown-engine/network/network_world_client.cpp
src/paintown-engine/network/server.cpp)

set(ENV_SRC
src/paintown-engine/environment/atmosphere.cpp)

set(FACTORY_SRC
src/factory/font_factory.cpp
src/paintown-engine/factory/heart_factory.cpp
src/paintown-engine/factory/object_factory.cpp
src/factory/font_render.cpp
src/factory/collector.cpp
src/paintown-engine/factory/shadow.cpp)

set(OBJECT_SRC
src/paintown-engine/object/actor.cpp
src/paintown-engine/object/animation.cpp
src/paintown-engine/object/animation_event.cpp
src/paintown-engine/object/animation_trail.cpp
src/paintown-engine/object/buddy_player.cpp
src/paintown-engine/object/cat.cpp
src/paintown-engine/object/display_character.cpp
src/paintown-engine/object/draw-effect.cpp
src/paintown-engine/object/effect.cpp
src/paintown-engine/object/enemy.cpp
src/paintown-engine/object/gib.cpp
src/paintown-engine/object/item.cpp
src/paintown-engine/object/network_character.cpp
src/paintown-engine/object/network_player.cpp
src/paintown-engine/object/object.cpp
src/paintown-engine/object/object_attack.cpp
src/paintown-engine/object/object_nonattack.cpp
src/paintown-engine/object/player.cpp
src/paintown-engine/object/player-common.cpp
src/paintown-engine/object/versus_player.cpp
src/paintown-engine/object/versus_enemy.cpp
src/paintown-engine/object/projectile.cpp
src/paintown-engine/object/attack.cpp
src/paintown-engine/object/stimulation.cpp
src/paintown-engine/object/health_stimulation.cpp
src/paintown-engine/object/character.cpp
src/paintown-engine/object/heart.cpp)

set(UTIL_SRC
src/util/token.cpp
src/util/token_exception.cpp
src/util/resource.cpp
src/util/tokenreader.cpp
src/util/timedifference.cpp
src/util/debug.cpp
src/util/sound.cpp
src/util/timer.cpp
src/util/music.cpp
src/util/init.cpp
src/util/input/keyboard.cpp
src/util/loading.cpp
src/util/messages.cpp
src/util/bitmap.cpp
src/util/events.cpp
src/util/font.cpp
src/util/fire.cpp
src/util/ftalleg.cpp
src/util/funcs.cpp
src/util/file-system.cpp
src/util/gradient.cpp
src/util/music-player.cpp
src/util/ebox.cpp
src/util/regex.cpp
src/util/language-string.cpp
src/util/thread.cpp
src/util/input/joystick.cpp
src/util/input/allegro5/joystick.cpp
src/util/input/allegro/allegro-joystick.cpp
src/util/input/sdl/joystick.cpp
src/util/input/psp/joystick.cpp
src/util/input/wii/joystick.cpp
src/util/system.cpp
src/util/compress.cpp
src/util/audio.cpp
src/util/message-queue.cpp
src/util/input/linux_joystick.cpp
src/util/load_exception.cpp
src/util/windows/funcs.cpp
src/util/windows/system.cpp
src/util/nacl/module.cpp
src/util/nacl/network-system.cpp
src/exceptions/exception.cpp
src/openbor/pack-reader.cpp
src/openbor/animation.cpp
src/openbor/display-character.cpp
src/openbor/mod.cpp
src/openbor/util.cpp
src/openbor/data.cpp)

set(LOADPNG_SRC
src/util/loadpng/loadpng.c
src/util/loadpng/regpng.c
src/util/loadpng/savepng.c)

set(MENU_SRC
src/menu/actionfactory.cpp
src/menu/action_speed.cpp
src/menu/menu.cpp
src/menu/font-info.cpp
src/menu/menu_action.cpp
src/menu/menu_option.cpp
src/menu/options.cpp
src/menu/optionfactory.cpp
)

set(GUI_SRC
src/util/gui/al_keyinput.cpp
src/util/gui/animation.cpp
src/util/gui/box.cpp
src/util/gui/container.cpp
src/util/gui/context-box.cpp
src/util/gui/coordinate.cpp
src/util/gui/fadetool.cpp
src/util/gui/keys.cpp
src/util/gui/keyinput.cpp
src/util/gui/keyinput_manager.cpp
src/util/gui/lineedit.cpp
src/util/gui/rectarea.cpp
src/util/gui/popup-box.cpp
src/util/gui/scroll-list.cpp
src/util/gui/tabbed-box.cpp
src/util/gui/timer.cpp
src/util/gui/widget.cpp
)

set(MUGEN_SRC
src/mugen/background.cpp
src/mugen/behavior.cpp
src/mugen/characterhud.cpp
src/mugen/character.cpp
src/mugen/character-select.cpp
src/mugen/config.cpp
src/mugen/evaluator.cpp
src/mugen/compiler.cpp
src/mugen/helper.cpp
src/mugen/game.cpp
src/mugen/command.cpp
src/mugen/storyboard.cpp
src/mugen/state.cpp
src/mugen/animation.cpp
src/mugen/exception.cpp
src/mugen/font.cpp
src/mugen/item.cpp
src/mugen/item-content.cpp
src/mugen/menu.cpp
src/mugen/reader.cpp
src/mugen/section.cpp
src/mugen/sound.cpp
src/mugen/sprite.cpp
src/mugen/stage.cpp
src/mugen/util.cpp
src/mugen/search.cpp
src/mugen/state-controller.cpp
src/mugen/option-options.cpp
src/mugen/ast/ast.cpp
src/mugen/parse-cache.cpp
src/mugen/parser/parse-exception.cpp
src/mugen/parser/def.cpp
src/mugen/parser/cmd.cpp
src/mugen/parser/air.cpp
)

set(PLATFORMER_SRC
src/platformer/game/animation.cpp
src/platformer/game/background.cpp
src/platformer/game/camera.cpp
src/platformer/game/cutscene.cpp
src/platformer/game/game.cpp
src/platformer/game/tile.cpp
src/platformer/game/world.cpp
src/platformer/object/object.cpp
src/platformer/script/script.cpp
)

set(TRIGGER_SRC
src/paintown-engine/trigger/trigger.cpp
)
