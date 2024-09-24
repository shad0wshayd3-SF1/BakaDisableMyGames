class Hooks
{
public:
	static void Install()
	{
		hkPhotoModePath<130787, 0x082>::Install();
		hkPhotoModePath<139670, 0x32C>::Install();
		hkPhotoModePath<139689, 0x2A7>::Install();
		hkPhotoModePath<139722, 0x023>::Install();

		hkMessageOfTheDayPath<134324, 0x239>::Install();
		hkMessageOfTheDayPath<134326, 0x14B>::Install();

		hkDisableLooseFileLocation<211739, 0x175>::Install();

		detail::BuildPath();
	}

private:
	class detail
	{
	public:
		static void BuildPath()
		{
			std::string path_t;
			path_t.resize(260);
			[[maybe_unused]] auto result =
				RE::WinAPI::GetCurrentDirectory(
					static_cast<std::uint32_t>(path_t.size()),
					path_t.data());

			path.clear();
			path.append(path_t.data());
			path.append("\\");
		}

		inline static std::string path;
	};

	template <std::uintptr_t ID, std::ptrdiff_t OFF>
	class hkPhotoModePath
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::ID(ID), OFF };
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

	template <std::uintptr_t ID, std::ptrdiff_t OFF>
	class hkMessageOfTheDayPath
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::ID(ID), OFF };
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

	template <std::uintptr_t ID, std::ptrdiff_t OFF>
	class hkDisableLooseFileLocation
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::ID(ID), OFF };
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

	const auto plugin = SFSE::PluginVersionData::GetSingleton();
	SFSE::log::info("{} {} loaded", plugin->GetPluginName(), plugin->GetPluginVersion());

	SFSE::AllocTrampoline(128);
	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}
