#pragma once

namespace Myo_app {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for SetInputDialog
	/// </summary>
	public ref class SetKeyOutputDialog : public System::Windows::Forms::Form
	{
	public: Keys keycode;
	public:
		SetKeyOutputDialog(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~SetKeyOutputDialog()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Button^  buttonCancel;
	private: System::Windows::Forms::Button^  buttonAccept;
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::TextBox^  textBoxKeyPress;
	private: System::Windows::Forms::Label^  label2;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->buttonCancel = (gcnew System::Windows::Forms::Button());
			this->buttonAccept = (gcnew System::Windows::Forms::Button());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->textBoxKeyPress = (gcnew System::Windows::Forms::TextBox());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->SuspendLayout();
			// 
			// buttonCancel
			// 
			this->buttonCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->buttonCancel->Location = System::Drawing::Point(138, 114);
			this->buttonCancel->Name = L"buttonCancel";
			this->buttonCancel->Size = System::Drawing::Size(120, 26);
			this->buttonCancel->TabIndex = 2;
			this->buttonCancel->TabStop = false;
			this->buttonCancel->Text = L"Cancel";
			this->buttonCancel->UseVisualStyleBackColor = true;
			this->buttonCancel->Click += gcnew System::EventHandler(this, &SetKeyOutputDialog::buttonCancel_Click);
			// 
			// buttonAccept
			// 
			this->buttonAccept->Enabled = false;
			this->buttonAccept->Location = System::Drawing::Point(12, 114);
			this->buttonAccept->Name = L"buttonAccept";
			this->buttonAccept->Size = System::Drawing::Size(120, 26);
			this->buttonAccept->TabIndex = 1;
			this->buttonAccept->TabStop = false;
			this->buttonAccept->Text = L"Accept";
			this->buttonAccept->UseVisualStyleBackColor = true;
			this->buttonAccept->Click += gcnew System::EventHandler(this, &SetKeyOutputDialog::buttonAccept_Click);
			// 
			// label1
			// 
			this->label1->Anchor = System::Windows::Forms::AnchorStyles::Top;
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(50, 61);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(25, 13);
			this->label1->TabIndex = 0;
			this->label1->Text = L"Key";
			// 
			// textBoxKeyPress
			// 
			this->textBoxKeyPress->Anchor = System::Windows::Forms::AnchorStyles::Top;
			this->textBoxKeyPress->BackColor = System::Drawing::SystemColors::Window;
			this->textBoxKeyPress->Location = System::Drawing::Point(81, 58);
			this->textBoxKeyPress->Name = L"textBoxKeyPress";
			this->textBoxKeyPress->ReadOnly = true;
			this->textBoxKeyPress->Size = System::Drawing::Size(136, 20);
			this->textBoxKeyPress->TabIndex = 0;
			this->textBoxKeyPress->TabStop = false;
			// 
			// label2
			// 
			this->label2->Anchor = System::Windows::Forms::AnchorStyles::Top;
			this->label2->AutoSize = true;
			this->label2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label2->Location = System::Drawing::Point(78, 29);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(105, 13);
			this->label2->TabIndex = 0;
			this->label2->Text = L"Press key to emulate";
			this->label2->TextAlign = System::Drawing::ContentAlignment::TopCenter;
			// 
			// SetKeyOutputDialog
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::SystemColors::Window;
			this->ClientSize = System::Drawing::Size(270, 152);
			this->ControlBox = false;
			this->Controls->Add(this->label2);
			this->Controls->Add(this->textBoxKeyPress);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->buttonAccept);
			this->Controls->Add(this->buttonCancel);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->KeyPreview = true;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"SetKeyOutputDialog";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Set Key Output";
			this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &SetKeyOutputDialog::SetKeyOutputDialog_KeyDown);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
		private: System::Void buttonCancel_Click(System::Object^  sender, System::EventArgs^  e) {
		}
		private: System::Void buttonAccept_Click(System::Object^  sender, System::EventArgs^  e) {
			this->Close();
			this->DialogResult = System::Windows::Forms::DialogResult::OK;
		}
		private: System::Void SetKeyOutputDialog_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e) {
			this->keycode = e->KeyCode;
			KeysConverter converter;
			textBoxKeyPress->Text = converter.ConvertToString(e->KeyCode);
			buttonAccept->Enabled = true;
		}
	};
}
