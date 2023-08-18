#include "NEGUI2/Ui/TextEditDemo.hpp"
#include <fstream>

namespace NEGUI2
{

    TextEditDemo::TextEditDemo()
        : IUserInterface::IUserInterface(), editor_(), file_to_edit_()
    {
        auto lang = TextEditor::LanguageDefinition::CPlusPlus();
        // set your own known preprocessor symbols...
        static const char *ppnames[] = {"NULL", "PM_REMOVE",
                                        "ZeroMemory", "DXGI_SWAP_EFFECT_DISCARD", "D3D_FEATURE_LEVEL", "D3D_DRIVER_TYPE_HARDWARE", "WINAPI", "D3D11_SDK_VERSION", "assert"};
        // ... and their corresponding values
        static const char *ppvalues[] = {
            "#define NULL ((void*)0)",
            "#define PM_REMOVE (0x0001)",
            "Microsoft's own memory zapper function\n(which is a macro actually)\nvoid ZeroMemory(\n\t[in] PVOID  Destination,\n\t[in] SIZE_T Length\n); ",
            "enum DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD = 0",
            "enum D3D_FEATURE_LEVEL",
            "enum D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE  = ( D3D_DRIVER_TYPE_UNKNOWN + 1 )",
            "#define WINAPI __stdcall",
            "#define D3D11_SDK_VERSION (7)",
            " #define assert(expression) (void)(                                                  \n"
            "    (!!(expression)) ||                                                              \n"
            "    (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0) \n"
            " )"};

        for (int i = 0; i < sizeof(ppnames) / sizeof(ppnames[0]); ++i)
        {
            TextEditor::Identifier id;
            id.mDeclaration = ppvalues[i];
            lang.mPreprocIdentifiers.insert(std::make_pair(std::string(ppnames[i]), id));
        }

        // set your own identifiers
        static const char *identifiers[] = {
            "HWND", "HRESULT", "LPRESULT", "D3D11_RENDER_TARGET_VIEW_DESC", "DXGI_SWAP_CHAIN_DESC", "MSG", "LRESULT", "WPARAM", "LPARAM", "UINT", "LPVOID",
            "ID3D11Device", "ID3D11DeviceContext", "ID3D11Buffer", "ID3D11Buffer", "ID3D10Blob", "ID3D11VertexShader", "ID3D11InputLayout", "ID3D11Buffer",
            "ID3D10Blob", "ID3D11PixelShader", "ID3D11SamplerState", "ID3D11ShaderResourceView", "ID3D11RasterizerState", "ID3D11BlendState", "ID3D11DepthStencilState",
            "IDXGISwapChain", "ID3D11RenderTargetView", "ID3D11Texture2D", "TextEditor"};
        static const char *idecls[] =
            {
                "typedef HWND_* HWND", "typedef long HRESULT", "typedef long* LPRESULT", "struct D3D11_RENDER_TARGET_VIEW_DESC", "struct DXGI_SWAP_CHAIN_DESC",
                "typedef tagMSG MSG\n * Message structure", "typedef LONG_PTR LRESULT", "WPARAM", "LPARAM", "UINT", "LPVOID",
                "ID3D11Device", "ID3D11DeviceContext", "ID3D11Buffer", "ID3D11Buffer", "ID3D10Blob", "ID3D11VertexShader", "ID3D11InputLayout", "ID3D11Buffer",
                "ID3D10Blob", "ID3D11PixelShader", "ID3D11SamplerState", "ID3D11ShaderResourceView", "ID3D11RasterizerState", "ID3D11BlendState", "ID3D11DepthStencilState",
                "IDXGISwapChain", "ID3D11RenderTargetView", "ID3D11Texture2D", "class TextEditor"};
        for (int i = 0; i < sizeof(identifiers) / sizeof(identifiers[0]); ++i)
        {
            TextEditor::Identifier id;
            id.mDeclaration = std::string(idecls[i]);
            lang.mIdentifiers.insert(std::make_pair(std::string(identifiers[i]), id));
        }
        editor_.SetLanguageDefinition(lang);
        // editor_.SetPalette(TextEditor::GetLightPalette());

        // error markers
        TextEditor::ErrorMarkers markers;
        markers.insert(std::make_pair<int, std::string>(6, "Example error here:\nInclude file not found: \"TextEditor.h\""));
        markers.insert(std::make_pair<int, std::string>(41, "Another example error"));
        editor_.SetErrorMarkers(markers);

        // "breakpoint" markers
        // TextEditor::Breakpoints bpts;
        // bpts.insert(24);
        // bpts.insert(47);
        // editor_.SetBreakpoints(bpts);

        file_to_edit_ = "ImGuiColorTextEdit/TextEditor.cpp";
        //	static const char* file_to_edit_ = "test.cpp";

        {
            std::ifstream t(file_to_edit_);
            if (t.good())
            {
                std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
                editor_.SetText(str);
            }
        }
    }

    TextEditDemo::~TextEditDemo()
    {
    }

    void TextEditDemo::update()
    {

		ImGui::ShowMetricsWindow();
        auto cpos = editor_.GetCursorPosition();
        ImGui::Begin("Text Editor Demo", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
        ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Save"))
                {
                    auto textToSave = editor_.GetText();
                    /// save text....
                }
                if (ImGui::MenuItem("Quit", "Alt-F4"))
                    return;
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                bool ro = editor_.IsReadOnly();
                if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
                    editor_.SetReadOnly(ro);
                ImGui::Separator();

                if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor_.CanUndo()))
                    editor_.Undo();
                if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor_.CanRedo()))
                    editor_.Redo();

                ImGui::Separator();

                if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor_.HasSelection()))
                    editor_.Copy();
                if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor_.HasSelection()))
                    editor_.Cut();
                if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor_.HasSelection()))
                    editor_.Delete();
                if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
                    editor_.Paste();

                ImGui::Separator();

                if (ImGui::MenuItem("Select all", nullptr, nullptr))
                    editor_.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor_.GetTotalLines(), 0));

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("Dark palette"))
                    editor_.SetPalette(TextEditor::GetDarkPalette());
                if (ImGui::MenuItem("Light palette"))
                    editor_.SetPalette(TextEditor::GetLightPalette());
                if (ImGui::MenuItem("Retro blue palette"))
                    editor_.SetPalette(TextEditor::GetRetroBluePalette());
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor_.GetTotalLines(),
                    editor_.IsOverwrite() ? "Ovr" : "Ins",
                    editor_.CanUndo() ? "*" : " ",
                    editor_.GetLanguageDefinition().mName.c_str(), file_to_edit_.c_str());

        editor_.Render("TextEditor");
        ImGui::End();
    }

}
