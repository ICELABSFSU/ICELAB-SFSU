#pragma once

namespace Myo_app {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for SetBitOutputDialog
	/// </summary>
	public ref class SetBitOutputDialog : public System::Windows::Forms::Form
	{
	public:
		SetBitOutputDialog(void)
		{
			InitializeComponent();

			// Fill in the bit assignment comboBox: 7-0
			comboBoxBitSelect->Items->Clear();
			comboBoxBitSelect->Items->Add(L"0 (LSB)");
			for (int i = 1; i < 7; i++)
				comboBoxBitSelect->Items->Add(L"" + i);
			comboBoxBitSelect->Items->Add(L"7 (MSB)");
			comboBoxBitSelect->SelectedIndex = 0;
		}
	private: System::Windows::Forms::ComboBox^  comboBoxBitSelect;
	public:
	public: int bit;

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~SetBitOutputDialog()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Button^  buttonCancel;
	private: System::Windows::Forms::Button^  buttonAccept;
	private: System::Windows::Forms::Label^  label1;



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
			this->comboBoxBitSelect = (gcnew System::Windows::Forms::ComboBox());
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
			this->buttonCancel->Click += gcnew System::EventHandler(this, &SetBitOutputDialog::buttonCancel_Click);
			// 
			// buttonAccept
			// 
			this->buttonAccept->Enabled = true;
			this->buttonAccept->Location = System::Drawing::Point(12, 114);
			this->buttonAccept->Name = L"buttonAccept";
			this->buttonAccept->Size = System::Drawing::Size(120, 26);
			this->buttonAccept->TabIndex = 1;
			this->buttonAccept->TabStop = false;
			this->buttonAccept->Text = L"Accept";
			this->buttonAccept->UseVisualStyleBackColor = true;
			this->buttonAccept->Click += gcnew System::EventHandler(this, &SetBitOutputDialog::buttonAccept_Click);
			// 
			// label1
			// 
			this->label1->Anchor = System::Windows::Forms::AnchorStyles::Top;
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(50, 61);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(19, 13);
			this->label1->TabIndex = 0;
			this->label1->Text = L"Bit";
			// 
			// comboBoxBitSelect
			// 
			this->comboBoxBitSelect->FormattingEnabled = true;
			this->comboBoxBitSelect->Location = System::Drawing::Point(75, 58);
			this->comboBoxBitSelect->Name = L"comboBoxBitSelect";
			this->comboBoxBitSelect->Size = System::Drawing::Size(121, 21);
			this->comboBoxBitSelect->TabIndex = 3;
			// 
			// SetBitOutputDialog
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::SystemColors::Window;
			this->ClientSize = System::Drawing::Size(270, 152);
			this->ControlBox = false;
			this->Controls->Add(this->comboBoxBitSelect);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->buttonAccept);
			this->Controls->Add(this->buttonCancel);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->KeyPreview = true;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"SetBitOutputDialog";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Set Bit Output";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void buttonCancel_Click(System::Object^  sender, System::EventArgs^  e) {
	}
	private: System::Void buttonAccept_Click(System::Object^  sender, System::EventArgs^  e) {
		this->bit = comboBoxBitSelect->SelectedIndex;
		this->Close();
		this->DialogResult = System::Windows::Forms::DialogResult::OK;
	}
	};
}
