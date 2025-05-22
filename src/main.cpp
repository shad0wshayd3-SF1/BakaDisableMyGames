namespace Hooks
{
	namespace detail
	{
		static auto GetPath()
		{
			static std::string path;
			if (path.empty())
			{
				auto root = std::filesystem::current_path();
				path = root.make_preferred().string() + '\\';
			}

			return path.c_str();
		}

		static auto GetPath(std::string_view a_append)
		{
			static std::string path;
			if (path.empty())
			{
				auto root = std::filesystem::current_path();
				root /= a_append;
				path = root.make_preferred().string();
			}

			return path.c_str();
		}
	}

	namespace hkDisableLooseFileLocation
	{
		static errno_t DisableLooseFileLocation(char* a_destination, std::size_t a_size, const char*)
		{
			auto path = detail::GetPath();
			return strcpy_s(a_destination, a_size, path);
		}

		// BSWinPCGameDataSystemUtility
		inline static REL::Hook _Hook0{ REL::ID(150201), 0x1C2, DisableLooseFileLocation };
	}

	namespace hkMessageOfTheDayPath
	{
		static void MessageOfTheDayPath(char* a_destination)
		{
			auto path = detail::GetPath("Data/Textures/MOTD_Media/"sv);
			strcpy_s(a_destination, 260, path);
		}

		// Data/Textures/Motd_Media/
		inline static REL::Hook _Hook0{ REL::ID(87384), 0x168, MessageOfTheDayPath };
		inline static REL::Hook _Hook1{ REL::ID(87387), 0x0F3, MessageOfTheDayPath };
	}

	class hkPhotoModePath :
		public REX::Singleton<hkPhotoModePath>
	{
	private:
		static void PhotoModePath(char* a_destination, std::size_t a_size, const char* a_source, const char*, const char* a_arg)
		{
			auto path = detail::GetPath();
			return _Hook0(a_destination, a_size, a_source, path, a_arg);
		}

		// Data\\Textures\\Photos
		inline static REL::Hook _Hook0{ REL::ID(84430), 0x316, PhotoModePath };
		inline static REL::Hook _Hook1{ REL::ID(84457), 0x085, PhotoModePath };
		inline static REL::Hook _Hook2{ REL::ID(92021), 0x04E, PhotoModePath };
		inline static REL::Hook _Hook3{ REL::ID(114073), 0x3D8, PhotoModePath };
	};
};

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse)
{
	SFSE::Init(a_sfse, { .trampoline = true });
	return true;
}
