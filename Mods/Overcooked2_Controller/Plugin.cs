using BepInEx;
using BepInEx.Logging;
using HarmonyLib;
using UnityEngine;
using System.Reflection;
using System.Collections.Generic;

namespace Overcooked2_Controller{
  
	[BepInPlugin(PluginInfo.PLUGIN_GUID, PluginInfo.PLUGIN_NAME, PluginInfo.PLUGIN_VERSION)]
	public class Plugin : BaseUnityPlugin {
	    public static new ManualLogSource Logger;
	
	    private void Awake() {
	        // Plugin startup logic
	        Logger = base.Logger;
	        Logger.LogInfo($"Plugin {PluginInfo.PLUGIN_GUID} is loaded!");
	        Harmony harmony = new Harmony(PluginInfo.PLUGIN_GUID);
			harmony.PatchAll();
	    }
	}

	[HarmonyPatch]
	internal static class HookExtraCheckForControls { 
		
	    [HarmonyTargetMethods]
		public static IEnumerable<MethodBase> GetTargetMethods() {
		    yield return AccessTools.Method(typeof(PlayerControls.ControlSchemeData), nameof(PlayerControls.ControlSchemeData.IsUseJustPressed));
			yield return AccessTools.Method(typeof(PlayerControls.ControlSchemeData), nameof(PlayerControls.ControlSchemeData.IsUseDown));
		}

		private static bool Prefix(PlayerControls.ControlSchemeData __instance) {
			var m_controlsProperty = __instance.GetType().GetField("m_controls", BindingFlags.NonPublic | BindingFlags.Instance);
			PlayerControls controls = (PlayerControls)m_controlsProperty.GetValue(__instance);
			ClientInteractable interactable = controls?.CurrentInteractionObjects?.m_interactable;
			if (interactable != null) {
				var asString = interactable.ToString().ToLower();
				if (asString.Contains("chopping")) {
					return IsChopping(__instance.Player);
				}
				Plugin.Logger.LogInfo(asString);
				if (asString.Contains("extinguisher")) {
					return IsUsingExtinguisher(__instance.Player);
				}
				if (asString.Contains("wash")) {
					return IsWashing(__instance.Player);
				}
			}
			return true;
		}
		private static bool IsChopping(PlayerInputLookup.Player player) => player switch {
			PlayerInputLookup.Player.One => Input.GetKey(KeyCode.C),
			PlayerInputLookup.Player.Two => Input.GetKey(KeyCode.B),
			_ => true
		};
		private static bool IsUsingExtinguisher(PlayerInputLookup.Player player) => true;
		private static bool IsWashing(PlayerInputLookup.Player player) => true;
	}

}
