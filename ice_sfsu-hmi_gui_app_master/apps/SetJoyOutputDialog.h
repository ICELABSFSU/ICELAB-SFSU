#pragma once
#include "GestureOutput.h"
#include <msclr\marshal_cppstd.h>
#include <map>
#include <string>

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
	public ref class SetJoyOutputDialog : public System::Windows::Forms::Form
	{
	public:
		int axis;
		int value;
	private:
		std::map<std::string, int>* axes;
		std::string* buttons;

		Void initializeAxesMap() {
			axes = new std::map<std::string, int>;
			axes->emplace("X", HID_USAGE_X);
			axes->emplace("Y", HID_USAGE_Y);
			axes->emplace("Z", HID_USAGE_Z);
			axes->emplace("RX", HID_USAGE_RX);
			axes->emplace("RY", HID_USAGE_RY);
			axes->emplace("RZ", HID_USAGE_RZ);
			axes->emplace("SL0", HID_USAGE_SL0);
			axes->emplace("SL1", HID_USAGE_SL1);
		}

		Void fillComboBoxAxes()
		{
			comboBox1->Items->Clear();
			for (auto it = axes->begin(); it != axes->end(); it++)
			{
				comboBox1->Items->Add(gcnew String(it->first.c_str()));
			}
			comboBox1->SelectedIndex = 0;
		}

		Void initializeButtonsMap() {
			buttons = new std::string[NUM_SETTABLE_BUTTONS];
			for (int i = 0; i < NUM_SETTABLE_BUTTONS; i++)
			{
				buttons[i] = "Button" + std::to_string(i+1);
			}
		}

		Void fillComboBoxButtons()
		{
			comboBox1->Items->Clear();
			for (int i = 0; i < NUM_SETTABLE_BUTTONS; i++)
			{
				comboBox1->Items->Add(gcnew String(buttons[i].c_str()));
			}
			comboBox1->SelectedIndex = 0;
		}

	public:
		SetJoyOutputDialog(void)
		{
			InitializeComponent();
			initializeAxesMap();
			initializeButtonsMap();
			fillComboBoxButtons();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~SetJoyOutputDialog()
		{
			delete axes;
			delete[] buttons;
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Button^  buttonCancel;
	private: System::Windows::Forms::Button^  buttonAccept;
	private: System::Windows::Forms::RadioButton^  radioButtonButtons;
	private: System::Windows::Forms::RadioButton^  radioButtonAxes;
	private: System::Windows::Forms::ComboBox^  comboBox1;
	private: System::Windows::Forms::Label^  labelAxisValue;
	private: System::Windows::Forms::TextBox^  textBoxAxisValue;


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
			this->radioButtonButtons = (gcnew System::Windows::Forms::RadioButton());
			this->radioButtonAxes = (gcnew System::Windows::Forms::RadioButton());
			this->comboBox1 = (gcnew System::Windows::Forms::ComboBox());
			this->labelAxisValue = (gcnew System::Windows::Forms::Label());
			this->textBoxAxisValue = (gcnew System::Windows::Forms::TextBox());
			this->SuspendLayout();
			// 
			// buttonCancel
			// 
			this->buttonCancel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->buttonCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->buttonCancel->Location = System::Drawing::Point(138, 114);
			this->buttonCancel->Name = L"buttonCancel";
			this->buttonCancel->Size = System::Drawing::Size(120, 26);
			this->buttonCancel->TabIndex = 5;
			this->buttonCancel->Text = L"Cancel";
			this->buttonCancel->UseVisualStyleBackColor = true;
			this->buttonCancel->Click += gcnew System::EventHandler(this, &SetJoyOutputDialog::buttonCancel_Click);
			// 
			// buttonAccept
			// 
			this->buttonAccept->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->buttonAccept->Location = System::Drawing::Point(12, 114);
			this->buttonAccept->Name = L"buttonAccept";
			this->buttonAccept->Size = System::Drawing::Size(120, 26);
			this->buttonAccept->TabIndex = 4;
			this->buttonAccept->Text = L"Accept";
			this->buttonAccept->UseVisualStyleBackColor = true;
			this->buttonAccept->Click += gcnew System::EventHandler(this, &SetJoyOutputDialog::buttonAccept_Click);
			// 
			// radioButtonButtons
			// 
			this->radioButtonButtons->AutoSize = true;
			this->radioButtonButtons->Checked = true;
			this->radioButtonButtons->Location = System::Drawing::Point(65, 12);
			this->radioButtonButtons->Name = L"radioButtonButtons";
			this->radioButtonButtons->Size = System::Drawing::Size(56, 17);
			this->radioButtonButtons->TabIndex = 0;
			this->radioButtonButtons->TabStop = true;
			this->radioButtonButtons->Text = L"Button";
			this->radioButtonButtons->UseVisualStyleBackColor = true;
			this->radioButtonButtons->CheckedChanged += gcnew System::EventHandler(this, &SetJoyOutputDialog::radioButtonButtons_CheckedChanged);
			// 
			// radioButtonAxes
			// 
			this->radioButtonAxes->AutoSize = true;
			this->radioButtonAxes->Location = System::Drawing::Point(127, 12);
			this->radioButtonAxes->Name = L"radioButtonAxes";
			this->radioButtonAxes->Size = System::Drawing::Size(74, 17);
			this->radioButtonAxes->TabIndex = 1;
			this->radioButtonAxes->Text = L"Axis Value";
			this->radioButtonAxes->UseVisualStyleBackColor = true;
			this->radioButtonAxes->CheckedChanged += gcnew System::EventHandler(this, &SetJoyOutputDialog::radioButtonAxes_CheckedChanged);
			// 
			// comboBox1
			// 
			this->comboBox1->AutoCompleteMode = System::Windows::Forms::AutoCompleteMode::Suggest;
			this->comboBox1->AutoCompleteSource = System::Windows::Forms::AutoCompleteSource::ListItems;
			this->comboBox1->FormattingEnabled = true;
			this->comboBox1->Location = System::Drawing::Point(53, 38);
			this->comboBox1->Name = L"comboBox1";
			this->comboBox1->Size = System::Drawing::Size(164, 21);
			this->comboBox1->TabIndex = 2;
			// 
			// labelAxisValue
			// 
			this->labelAxisValue->AutoSize = true;
			this->labelAxisValue->Enabled = false;
			this->labelAxisValue->Location = System::Drawing::Point(50, 75);
			this->labelAxisValue->Name = L"labelAxisValue";
			this->labelAxisValue->Size = System::Drawing::Size(56, 13);
			this->labelAxisValue->TabIndex = 0;
			this->labelAxisValue->Text = L"Axis Value";
			// 
			// textBoxAxisValue
			// 
			this->textBoxAxisValue->Enabled = false;
			this->textBoxAxisValue->Location = System::Drawing::Point(112, 72);
			this->textBoxAxisValue->Name = L"textBoxAxisValue";
			this->textBoxAxisValue->Size = System::Drawing::Size(105, 20);
			this->textBoxAxisValue->TabIndex = 3;
			// 
			// SetJoyOutputDialog
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::SystemColors::Window;
			this->CancelButton = this->buttonCancel;
			this->ClientSize = System::Drawing::Size(270, 152);
			this->ControlBox = false;
			this->Controls->Add(this->textBoxAxisValue);
			this->Controls->Add(this->labelAxisValue);
			this->Controls->Add(this->comboBox1);
			this->Controls->Add(this->radioButtonAxes);
			this->Controls->Add(this->radioButtonButtons);
			this->Controls->Add(this->buttonAccept);
			this->Controls->Add(this->buttonCancel);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->KeyPreview = true;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"SetJoyOutputDialog";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Set Joy Output";
			this->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &SetJoyOutputDialog::SetJoyOutputDialog_KeyPress);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
		private: System::Void acceptControl() {
			if (radioButtonButtons->Checked)
			{
				//set button
				axis = 0;
				value = comboBox1->SelectedIndex + 1;
			}
			else if (radioButtonAxes->Checked)
			{
				//set axis value
				axis = axes->at(msclr::interop::marshal_as<std::string>(comboBox1->Text));
				if (sscanf(msclr::interop::marshal_as<std::string>(textBoxAxisValue->Text).c_str(), "%d", &value) != 1)
				{
					return;
				}
				if (value > 32767 || value < 0)
				{
					MessageBox::Show("Axis value must be 0 - 32767",
						"Invalid Axis Value", MessageBoxButtons::OK,
						MessageBoxIcon::Asterisk);
					return;
				}
			}
			this->Close();
			this->DialogResult = System::Windows::Forms::DialogResult::OK;
		}
		private: System::Void buttonCancel_Click(System::Object^  sender, System::EventArgs^  e) {
		}
		private: System::Void buttonAccept_Click(System::Object^  sender, System::EventArgs^  e) {
			acceptControl();
		}
		private: System::Void radioButtonButtons_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			if (radioButtonButtons->Checked)
			{
				fillComboBoxButtons();
			}
			labelAxisValue->Enabled = false;
			textBoxAxisValue->Enabled = false;
		}
		private: System::Void radioButtonAxes_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			if (radioButtonAxes->Checked)
			{
				fillComboBoxAxes();
			}
			labelAxisValue->Enabled = true;
			textBoxAxisValue->Enabled = true;
		}
		private: System::Void SetJoyOutputDialog_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
			if (e->KeyChar == (char)'\r')
			{
				acceptControl();
				e->Handled = true;
			}
		}
	};
}
