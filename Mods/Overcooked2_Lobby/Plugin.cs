using BepInEx;
using BepInEx.Logging;
using HarmonyLib;
using System.Reflection;
using System.Collections.Generic;
using Team17.Online;
using System;
using System.Linq;
using UnityEngine;

namespace Overcooked2_Lobby {
  
	[BepInPlugin(PluginInfo.PLUGIN_GUID, PluginInfo.PLUGIN_NAME, PluginInfo.PLUGIN_VERSION)]
	public class Plugin : BaseUnityPlugin
	{
	    public static new ManualLogSource Logger;
	
	    private void Awake()
	    {
	        Logger = base.Logger;
	        Logger.LogInfo($"Plugin {PluginInfo.PLUGIN_GUID} is loaded!");
	        Harmony harmony = new Harmony(PluginInfo.PLUGIN_GUID);
			harmony.PatchAll();
			Invoke(nameof(SwitchToLobby), 25);
			// pad 1 is registered in the StartScreen patch
			Invoke(nameof(RegisterPad2), 17);
			Invoke(nameof(SplitPad1), 21);
			Invoke(nameof(SplitPad2), 19);
	    }
	    
	    /**
	     * returns the number of the next gamepad, excluding keyboard, or Count if not found
	     */
	    public static ControlPadInput.PadNum GamePad(int number) {
			foreach (var pad in Enum.GetValues(typeof(ControlPadInput.PadNum)).Cast<ControlPadInput.PadNum>()) {
				if (!PCPadInputProvider.IsKeyboard(pad)) {
					number--;
				}
				if (number == 0) {
					return pad;
				}
			}
			return ControlPadInput.PadNum.Count;
		}
	    	
		private void RegisterPad2() {
			var playerManager = GameUtils.RequireManagerInterface<IPlayerManager>();
			ServerUserSystem.UnlockEngagement();
			EngagmentCircumstances o_circumstances = null;
			playerManager.StartPadEngagement(GamePad(2), o_circumstances, null);
		}
	    		
		private void SplitPad1() {
			ServerUserSystem.SplitUser(EngagementSlot.One);
		}
		
		private void SplitPad2() {
			ServerUserSystem.SplitUser(EngagementSlot.Two);
		}
	    
	    private void SwitchToLobby() {
			new FrontendCoopTabOptions().OnCouchPlayClicked();
		}
	}

	[HarmonyPatch(typeof(StartScreenFlow), "Start")]
	internal static class ByPassStartScreen {
		static MethodInfo callbackMethod;
		static StartScreenFlow i;
		public static void Postfix(StartScreenFlow __instance) {
			i = __instance;
			callbackMethod = typeof(StartScreenFlow).GetMethod("OnEngagementFinished", BindingFlags.NonPublic | BindingFlags.Instance);
			__instance.StartCoroutine(RegisterPad1());
		}
		
		private static IEnumerator<WaitForSeconds> RegisterPad1() {
	        yield return new WaitForSeconds(2);
			var playerManager = GameUtils.RequireManagerInterface<IPlayerManager>();
			ServerUserSystem.UnlockEngagement();
			EngagmentCircumstances o_circumstances = null;
			playerManager.StartPadEngagement(Plugin.GamePad(1), o_circumstances, user => {
				try {
					Plugin.Logger.LogInfo(callbackMethod);
					callbackMethod.Invoke(i, new object[]{ user });
				} catch (Exception ex) {
					Plugin.Logger.LogError(ex);
				}
			});
		}


	}

//	[HarmonyPatch]
//	internal static class LogInfosAboutPads { 
//		
//	    [HarmonyTargetMethods]
//		public static IEnumerable<MethodBase> GetTargetMethods() {
//			yield return AccessTools.Method(typeof(StartScreenFlow), "OnEngagementFinished");
//			//yield return AccessTools.Method(typeof(PlayerManagerShared<>).MakeGenericType(typeof(GamepadUser)), "GetUnengagedSlot");
////			yield return AccessTools.Method(typeof(SteamPlayerManager), "PadEngineRoutine");
////			yield return AccessTools.Method(typeof(PCPlayerManager), "PadEngineRoutine");
////			yield return AccessTools.Method(typeof(ClientUserSystem), "OnEngagementChanged");
////			yield return AccessTools.Method(typeof(ClientUserSystem), nameof(ClientUserSystem.OnUserAdded));
////			yield return AccessTools.Method(typeof(ServerUserSystem), nameof(ServerUserSystem.AddUser));
//			//yield return AccessTools.Method(typeof(PlayerManagerShared), "CanEngage");
//		    //yield return AccessTools.Method(typeof(PlayerManager), "PadEngageRoutine");
////		    yield return AccessTools.Method(typeof(LobbyUIController), "OnEnable");
////		    yield return AccessTools.Method(typeof(LobbySetupInfo), "Awake");
//		}
//
//		private static void Prefix() {
//			Plugin.Logger.LogInfo(new System.Diagnostics.StackTrace());
//		}
//	}

}
