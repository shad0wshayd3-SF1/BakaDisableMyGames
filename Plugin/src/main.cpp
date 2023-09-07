class Hooks
{
public:
	static void Install()
	{
		hkFormatString<0x01FC0880, 0x082>::Install();
		hkFormatString<0x0218CF50, 0x31E>::Install();
		hkFormatString<0x0218E5F0, 0x29F>::Install();
		hkFormatString<0x0218F59C, 0x023>::Install();

		hkMOTDPatch<0x02073140, 0x2C1>::Install();
		hkMOTDPatch<0x02073AC0, 0x14B>::Install();
	}

	static void GetPath()
	{
		std::string temp;
		temp.resize(260);
		GetCurrentDirectoryA(temp.size(), temp.data());

		detail::path.clear();
		detail::path.append(temp.data());
		detail::path.append("\\");
	}

private:
	class detail
	{
	public:
		inline static std::string path;
	};

	template <std::uintptr_t ADDR, std::ptrdiff_t OFF>
	class hkFormatString
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::Offset(ADDR), OFF };
			auto& trampoline = SFSE::GetTrampoline();
			_FormatString = trampoline.write_call<5>(target.address(), FormatString);
		}

	private:
		static void FormatString(char* a_buffer, std::size_t a_size, char* a_format, char* a_arg1, char* a_arg2)
		{
			return _FormatString(
				a_buffer,
				a_size,
				a_format,
				detail::path.data(),
				a_arg2);
		}

		inline static REL::Relocation<decltype(&FormatString)> _FormatString;
	};

	template <std::uintptr_t ADDR, std::ptrdiff_t OFF>
	class hkMOTDPatch
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::Offset(ADDR), OFF };
			auto& trampoline = SFSE::GetTrampoline();
			_FormatString = trampoline.write_call<5>(target.address(), FormatString);
		}

	private:
		static void FormatString(char* a_buffer)
		{
			std::string data{ detail::path.data() };
			data.append("Data\\Textures\\MOTD_Media\\"sv);
			a_buffer = data.data();
		}

		inline static REL::Relocation<decltype(&FormatString)> _FormatString;
	};
};

DLLEXPORT constinit auto SFSEPlugin_Version = []() noexcept {
	SFSE::PluginVersionData data{};

	data.PluginVersion(Plugin::Version);
	data.PluginName(Plugin::NAME);
	data.AuthorName(Plugin::AUTHOR);
	data.UsesSigScanning(false);
	//data.UsesAddressLibrary(true);
	data.HasNoStructUse(true);
	//data.IsLayoutDependent(true);
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

DLLEXPORT bool SFSEAPI SFSEPlugin_Load(const SFSE::LoadInterface* a_sfse)
{
#ifndef NDEBUG
	while (!IsDebuggerPresent()) {
		Sleep(100);
	}
#endif

	SFSE::Init(a_sfse);

	DKUtil::Logger::Init(Plugin::NAME, std::to_string(Plugin::Version));

	INFO("{} v{} loaded", Plugin::NAME, Plugin::Version);

	SFSE::AllocTrampoline(1 << 10);
	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	Hooks::GetPath();

	return true;
}
