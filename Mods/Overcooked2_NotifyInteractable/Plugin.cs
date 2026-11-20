﻿using System.IO;
using BepInEx;
using BepInEx.Configuration;
using BepInEx.Logging;
using HarmonyLib;
using System.Collections.Generic;

namespace Overcooked2_NotifyInteractable {
  
	[BepInPlugin(PluginInfo.PLUGIN_GUID, PluginInfo.PLUGIN_NAME, PluginInfo.PLUGIN_VERSION)]
	public class Plugin : BaseUnityPlugin {
	    public static new ManualLogSource Logger;
	    public static ConfigEntry<string> filePrefix;
	
	    private void Awake()
	    {
			Logger = base.Logger;
			filePrefix = Config.Bind("Serial",      // The section under which the option is shown
                     "portPrefix",  // The key of the configuration option in the configuration file
                     "/dev/ttyACM", // The default value
                     "Serial port prefix where current interactables info are sent; will be suffixed by player number (0~3)");
            if (File.Exists(filePrefix.Value + '0')) {
				Logger.LogInfo("Will write to " + filePrefix.Value + "0 and so on." +
					"Use `stty -F " + filePrefix.Value + "0 raw sane -echo && cat " + filePrefix.Value + "0` to visualize controller logs");
			} else {
				Logger.LogError("No " + filePrefix.Value + "0 custom controller");
			}
	        Harmony harmony = new Harmony(PluginInfo.PLUGIN_GUID);
			harmony.PatchAll();
	        Logger.LogInfo($"Plugin {PluginInfo.PLUGIN_GUID} is loaded!");
	    }
	}
	
	[HarmonyPatch(typeof(PlayerControls), "UpdateInteractAnticipation")]
	internal static class LogInteractionObjects {
		
		private static void Prefix(PlayerControls __instance, InteractionType _type, object _participantCmptBefore, object _participantCmptAfter) {
			if (_type != InteractionType.Interact || _participantCmptBefore == _participantCmptAfter) {
				return;
			}
			var p = __instance.ControlScheme.Player;
			char player = p switch {
				PlayerInputLookup.Player.One => '0',
				PlayerInputLookup.Player.Two => '1',
				PlayerInputLookup.Player.Three => '2',
				PlayerInputLookup.Player.Four => '3',
				_ => '9'
			};
			Plugin.Logger.LogInfo(player);
			char action = 'N';
			if (_participantCmptAfter != null) {
				var name = _participantCmptAfter.ToString().ToLower();
				if (name.Contains("chop")) {
					action = 'C';
				} else if (name.Contains("wash")) {
					action = 'W';
				} else if (name.Contains("exting")) {
					action = 'E';
				} else {
					Plugin.Logger.LogInfo("Unknown action: " + name);
				}
			}
			Plugin.Logger.LogInfo(action);
			if (Plugin.filePrefix != null && File.Exists(Plugin.filePrefix.Value + player)) {
				try {
					// create a (write only) new filestream everytime to avoid sharing buffer
					using (FileStream stream = new FileStream(Plugin.filePrefix.Value + player, FileMode.Open, FileAccess.Write, FileShare.Read, 1)) {
						stream.WriteByte((byte)player);
						stream.WriteByte((byte)action);
					}
					Plugin.Logger.LogInfo("wrote " + player.ToString() + action.ToString());
				} catch (System.Exception e) {
					Plugin.Logger.LogError(e);
				}
			} else {
				Plugin.Logger.LogError("No file?");
			}
		}
	}
}
