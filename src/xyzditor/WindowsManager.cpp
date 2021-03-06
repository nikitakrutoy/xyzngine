#include "WindowsManager.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"
#include "SDL.h"

WindowsManager::WindowsManager(RenderEngine* renderEngine)
{
	m_pGameObjectEditor = new GameObjectEditor(renderEngine, "Property Editor");
	m_pSceneTreeWindow = new SceneTreeWindow(renderEngine, "Scene Tree");
	m_pDemoWindow = new DemoWindow(renderEngine, "Demo");
	m_CameraWindow = new CameraWindow(renderEngine, "Camera Settings");
}

void WindowsManager::Update()
{
	m_pSceneTreeWindow->Update();
	m_pGameObjectEditor->SetSelected(m_pSceneTreeWindow->GetSelected());
	m_pGameObjectEditor->Update();
	m_pDemoWindow->Update();
	m_CameraWindow->Update();
}

void WindowsManager::ProcessInput()
{
	SDL_Event ev;
	bool processed;
	std::vector<SDL_Event> notProcessedEvents;
	while (SDL_PollEvent(&ev))
	{
		processed = false;
		for (auto w : GetWindows()) {
			if (ev.window.windowID == SDL_GetWindowID(w->GetWindow())) {
				w->SetImguiContext();
				ImGui_ImplSDL2_ProcessEvent(&ev);

				if (ev.window.event == SDL_WINDOWEVENT_RESIZED) {
					int wd, ht;
					SDL_GetWindowSize(w->GetWindow(), &wd, &ht);
					w->SetSize(wd, ht);
				}

				processed = true;
				break;
			}
		}
		if (!processed) {
			notProcessedEvents.push_back(ev);
		}
	}

	for (auto ev : notProcessedEvents) {
		SDL_PushEvent(&ev);
	}
}
