class Config
{
public:
	class General
	{
	public:
		inline static DKUtil::Alias::Boolean bMO2Compatibility{ "bMO2Compatibility", "General" };
	};

	static void Load()
	{
		static auto MainConfig = COMPILE_PROXY("BakaKillMyGames.ini");
		MainConfig.Bind(General::bMO2Compatibility, false);
		MainConfig.Load();
	}
};

class Hooks
{
public:
	static void Install()
	{
		hkPhotoModePath<0x01FC0280, 0x082>::Install();
		hkPhotoModePath<0x0218FA50, 0x31E>::Install();
		hkPhotoModePath<0x021910F0, 0x29F>::Install();
		hkPhotoModePath<0x0219209C, 0x023>::Install();

		hkMessageOfTheDayPath<0x02074310, 0x2C1>::Install();
		hkMessageOfTheDayPath<0x02074C90, 0x14B>::Install();

		hkDisableLooseFileLocation<0x034BEE90, 0x172>::Install();
	}

	static void SetPath()
	{
		std::string path;
		path.resize(260);
		GetCurrentDirectoryA(path.size(), path.data());

		detail::path.clear();
		detail::path.append(path.data());
		detail::path.append("\\");
	}

	static std::string& GetPath()
	{
		return detail::path;
	}

private:
	class detail
	{
	public:
		inline static std::string path;
	};

	template <std::uintptr_t ADDR, std::ptrdiff_t OFF>
	class hkPhotoModePath
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::Offset(ADDR), OFF };
			auto& trampoline = SFSE::GetTrampoline();
			_PhotoModePath = trampoline.write_call<5>(target.address(), PhotoModePath);
		}

	private:
		static void PhotoModePath(
			[[maybe_unused]] char* a_destination,
			[[maybe_unused]] std::size_t a_size,
			[[maybe_unused]] const char* a_source,
			[[maybe_unused]] const char* a_vArg1,
			[[maybe_unused]] const char* a_vArg2)
		{
			return _PhotoModePath(a_destination, a_size, a_source, detail::path.data(), a_vArg2);
		}

		inline static REL::Relocation<decltype(&PhotoModePath)> _PhotoModePath;
	};

	template <std::uintptr_t ADDR, std::ptrdiff_t OFF>
	class hkMessageOfTheDayPath
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::Offset(ADDR), OFF };
			auto& trampoline = SFSE::GetTrampoline();
			trampoline.write_call<5>(target.address(), MessageOfTheDayPath);
		}

	private:
		static void MessageOfTheDayPath(
			[[maybe_unused]] char* a_destination)
		{
			auto path = std::filesystem::path{ detail::path };
			path /= "Data/Textures/MOTD_Media/"sv;
			strcpy_s(a_destination, 260, path.make_preferred().string().data());
		}
	};

	template <std::uintptr_t ADDR, std::ptrdiff_t OFF>
	class hkDisableLooseFileLocation
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::Offset(ADDR), OFF };
			auto& trampoline = SFSE::GetTrampoline();
			trampoline.write_call<5>(target.address(), DisableLooseFileLocation);
		}

	private:
		static errno_t DisableLooseFileLocation(
			[[maybe_unused]] char* a_destination,
			[[maybe_unused]] std::size_t a_size,
			[[maybe_unused]] const char* a_source)
		{
			auto path = std::filesystem::path{ detail::path };
			return strcpy_s(a_destination, a_size, path.make_preferred().string().data());
		}
	};
};

DLLEXPORT constinit auto SFSEPlugin_Version = []() noexcept {
	SFSE::PluginVersionData data{};

	data.PluginVersion(Plugin::Version);
	data.PluginName(Plugin::NAME);
	data.AuthorName(Plugin::AUTHOR);
	data.UsesSigScanning(false);
	data.UsesAddressLibrary(false);
	data.HasNoStructUse(true);
	data.IsLayoutDependent(false);
	data.CompatibleVersions({ SFSE::RUNTIME_LATEST });

	return data;
}();

namespace
{
	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept
	{
		switch (a_msg->type) {
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

DLLEXPORT void SFSEAPI SFSEPlugin_Preload(const SFSE::LoadInterface* a_sfse)
{
	SFSE::Init(a_sfse);

	DKUtil::Logger::Init(Plugin::NAME, std::to_string(Plugin::Version));

	INFO("{} v{} preloaded"sv, Plugin::NAME, Plugin::Version);

	Config::Load();
	Hooks::SetPath();

	if (!(*Config::General::bMO2Compatibility)) {
		PWSTR folderPath{ nullptr };
		if (SHGetKnownFolderPath(FOLDERID_Documents, KNOWN_FOLDER_FLAG::KF_FLAG_DEFAULT, NULL, &folderPath) == S_OK) {
			auto path = std::filesystem::path{ folderPath };
			path /= "My Games/Starfield/Data/"sv;

			if (std::filesystem::exists(path)) {
				auto data = std::filesystem::path{ Hooks::GetPath() };
				data /= "Data"sv;

				static constexpr auto MessageText = std::string_view{
					"\n\nA Data folder has been detected in Starfield's \"My Games\" folder."sv
					"\n\nWith this mod installed, the Data folder in Starfield's \"My Games\" folder is disabled. Files located there will not load, including Photo Mode photos shown in the Photo Gallery and during Load Screens."sv
					"\n\nThe path to this folder is:\n{}\n\nCopy any files contained in that Data folder to the game's Data folder, then delete the Data folder in \"My Games\"."sv
					"\n\nThe path to the game's Data folder is: \n{}"sv
					"\n\nWhen you have successfully done this, this message will stop appearing, and you will be able to launch the game."sv
					"\n\nIf you are using MO2 or the latest Vortex, but still want to use this mod, set bMO2Compatibility in this mod's .ini file to true."sv
				};

				auto msg = fmt::format(
					MessageText,
					path.make_preferred().string(),
					data.make_preferred().string());
				REL::stl::report_and_fail(msg);
			}
		}
	}
}

DLLEXPORT bool SFSEAPI SFSEPlugin_Load(const SFSE::LoadInterface* a_sfse)
{
#ifndef NDEBUG
	while (!IsDebuggerPresent()) {
		Sleep(100);
	}
#endif

	INFO("{} v{} loaded"sv, Plugin::NAME, Plugin::Version);

	SFSE::AllocTrampoline(1 << 7);
	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}
