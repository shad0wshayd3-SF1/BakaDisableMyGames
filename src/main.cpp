namespace Hooks
{
	namespace detail
	{
		static auto GetPath()
		{
			static std::string path;
			if (path.empty())
			{
				std::string path_t(REX::W32::MAX_PATH, 0);
				REX::W32::GetCurrentDirectoryA(static_cast<std::uint32_t>(path_t.size()), path_t.data());
				path.append(path_t.data());
				path.append("\\");
			}

			return path.c_str();
		}

		static auto GetPath(std::string_view a_append)
		{
			auto fs_path = std::filesystem::path{ GetPath() };
			fs_path /= a_append;

			static std::string path;
			path = fs_path.make_preferred().string();
			return path.c_str();
		}
	}

	namespace hkMessageOfTheDayPath
	{
		static void MessageOfTheDayPath(char* a_destination)
		{
			auto path = detail::GetPath("Data/Textures/MOTD_Media/"sv);
			strcpy_s(a_destination, 260, path);
		}

		template <std::uintptr_t ID, std::ptrdiff_t OFF>
		static void Install()
		{
			static REL::Relocation target{ REL::ID(ID), OFF };
			target.write_call<5>(MessageOfTheDayPath);
		}
	}

	namespace hkDisableLooseFileLocation
	{
		static errno_t DisableLooseFileLocation(char* a_destination, std::size_t a_size, const char*)
		{
			auto path = detail::GetPath();
			return strcpy_s(a_destination, a_size, path);
		}

		template <std::uintptr_t ID, std::ptrdiff_t OFF>
		static void Install()
		{
			static REL::Relocation target{ REL::ID(ID), OFF };
			target.write_call<5>(DisableLooseFileLocation);
		}
	}

	template <std::uintptr_t ID, std::ptrdiff_t OFF>
	class hkPhotoModePath :
		public REX::Singleton<hkPhotoModePath<ID, OFF>>
	{
	public:
		static void Install()
		{
			static REL::Relocation target{ REL::ID(ID), OFF };
			_PhotoModePath = target.write_call<5>(PhotoModePath);
		}

	private:
		static void PhotoModePath(char* a_destination, std::size_t a_size, const char* a_source, const char*, const char* a_vArg2)
		{
			auto path = detail::GetPath();
			return _PhotoModePath(a_destination, a_size, a_source, path, a_vArg2);
		}

		inline static REL::Relocation<decltype(&PhotoModePath)> _PhotoModePath;
	};

	static void Install()
	{
		hkPhotoModePath<130787, 0x082>::Install();
		hkPhotoModePath<139670, 0x32C>::Install();
		hkPhotoModePath<139689, 0x2A7>::Install();
		hkPhotoModePath<139722, 0x023>::Install();

		hkMessageOfTheDayPath::Install<134324, 0x239>();
		hkMessageOfTheDayPath::Install<134326, 0x14B>();

		hkDisableLooseFileLocation::Install<211739, 0x175>();
	}
};

namespace
{
	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept
	{
		switch (a_msg->type)
		{
		case SFSE::MessagingInterface::kPostLoad:
		{
			Hooks::Install();
			break;
		}
		default:
			break;
		}
	}
}

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse)
{
	SFSE::Init(a_sfse);

	SFSE::AllocTrampoline(128);
	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}
