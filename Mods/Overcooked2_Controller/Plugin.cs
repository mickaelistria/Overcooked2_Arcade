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
			PlayerInputLookup.Player.One => Input.GetKey(KeyCode.Alpha1),
			PlayerInputLookup.Player.Two => Input.GetKey(KeyCode.Alpha2),
			PlayerInputLookup.Player.Three => Input.GetKey(KeyCode.Alpha3),
			PlayerInputLookup.Player.Four => Input.GetKey(KeyCode.Alpha4),
			_ => true
		};
		private static bool IsUsingExtinguisher(PlayerInputLookup.Player player) => player switch {
			PlayerInputLookup.Player.One => Input.GetKey(KeyCode.W),
			PlayerInputLookup.Player.Two => Input.GetKey(KeyCode.X),
			PlayerInputLookup.Player.Three => Input.GetKey(KeyCode.Y),
			PlayerInputLookup.Player.Four => Input.GetKey(KeyCode.Z),
			_ => true
		};
		private static bool IsWashing(PlayerInputLookup.Player player) => player switch {
			PlayerInputLookup.Player.One => Input.GetKey(KeyCode.A),
			PlayerInputLookup.Player.Two => Input.GetKey(KeyCode.B),
			PlayerInputLookup.Player.Three => Input.GetKey(KeyCode.C),
			PlayerInputLookup.Player.Four => Input.GetKey(KeyCode.D),
			_ => true
		};
	}

}
