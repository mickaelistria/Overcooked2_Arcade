using System.IO;
using BepInEx;
using BepInEx.Configuration;
using BepInEx.Logging;
using HarmonyLib;
using UnityEngine;
using System.Threading;

namespace Overcooked2_NotifyInteractable {
  
	[BepInPlugin(PluginInfo.PLUGIN_GUID, PluginInfo.PLUGIN_NAME, PluginInfo.PLUGIN_VERSION)]
	public class Plugin : BaseUnityPlugin {
	    public static new ManualLogSource Logger;
	    public static FileStream serial;
	    public static ConfigEntry<string> file;
	
	    private void Awake()
	    {
			file = Config.Bind("Serial",      // The section under which the option is shown
                     "port",  // The key of the configuration option in the configuration file
                     "/dev/ttyACM0", // The default value
                     "Serial port where current interactables info are sent");
            Logger = base.Logger;
            if (File.Exists(file.Value)) {
				try {
//					serial = new FileStream(file.Value, 
//	                   FileMode.Open,
//	                   FileAccess.Read, 
//	                   FileShare.Write,
//	                   1);
//	                StreamReader reader = new StreamReader(serial);
//	                new Thread(() => {
//	             		while (true) {
//							Logger.LogInfo("From " + serial.Name + ": " + reader.ReadLine());
//						}
//					}).Start();
	                //serial.WriteByte((byte)'0'); serial.WriteByte((byte)'C');
	             } catch (System.Exception e) {
					Logger.LogError(e);
				 }
			} else {
				Logger.LogError("No " + file.Value + " custom controller");
			}
	        Harmony harmony = new Harmony(PluginInfo.PLUGIN_GUID);
			harmony.PatchAll();
	        Logger.LogInfo($"Plugin {PluginInfo.PLUGIN_GUID} is loaded!");
	    }
	}
	
	[HarmonyPatch(typeof(PlayerControls), "UpdateInteractAnticipation")]
	internal static class LogInteractionObjects {
		
		private static void Prefix(PlayerControls __instance, InteractionType _type, object _participantCmptBefore, object _participantCmptAfter) {
			if (_type == InteractionType.Interact && (_participantCmptBefore != _participantCmptAfter)) {
				char player = __instance.ControlScheme.Player switch {
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
					}
				}
				Plugin.Logger.LogInfo(action);
				if (Plugin.file != null) {
					try {
						// create a new filestream everytime to avoid sharing buffer
						using (FileStream stream = new FileStream(Plugin.file.Value, FileMode.Open, FileAccess.Write, FileShare.Read, 1)) {
							stream.WriteByte((byte)player);
							stream.WriteByte((byte)action);
						}
						Plugin.Logger.LogInfo("wrote " + player.ToString() + action.ToString());
					} catch (System.Exception e) {
						Plugin.Logger.LogError(e);
					}
				} else {
					Plugin.Logger.LogError("checlous");
				}
			}
		}
	}
}
