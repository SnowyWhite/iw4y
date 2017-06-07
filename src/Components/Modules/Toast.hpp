#pragma once

namespace Components
{
	class Toast : public Component
	{
	public:
		Toast();
		~Toast();

		static void Show(std::string image, std::string title, std::string description, int length, Utils::Slot<void()> callback = Utils::Slot<void()>());
		static void Show(Game::Material* material, std::string title, std::string description, int length, Utils::Slot<void()> callback = Utils::Slot<void()>());

		static std::string GetIcon();

	private:
		class UIToast
		{
		public:
			Game::Material* image;
			std::string title;
			std::string desc;
			int length;
			int start;
			Utils::Slot<void()> callback;
		};

		static void Handler();
		static void Draw(UIToast* toast);

		static std::queue<UIToast> Queue;
		static std::mutex Mutex;
	};
}
