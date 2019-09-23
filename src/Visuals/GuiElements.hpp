//
// Created by therdel on 07.09.19.
//
#pragma once

#include <string>
#include <functional>
#include <vector>

namespace GuiElements {
	struct IntSlider {
		int m_rangeBegin;
		int m_rangeEnd;
		int &m_value;
		std::string m_description;
	};

	struct FloatSlider {
		float m_rangeBegin;
		float m_rangeEnd;
		float &m_value;
		std::string m_description;
	};

	struct AngleRadSlider {
		float m_rangeBeginDegrees;
		float m_rangeEndDegrees;
		float &m_value;
		std::string m_description;
	};

	struct Checkbox {
		bool &m_value;
		std::string m_description;
	};

	struct Button {
		using Callback = std::function<void()>;

		Button(std::string description, Callback m_callback)
				: m_description(std::move(description))
				, m_callback(std::move(m_callback)) {}

		std::string m_description;
		Callback m_callback;
	};

	struct ComboBox {
		using Selection = std::vector<std::string>::const_iterator;
		using UpdateHandler = std::function<void(size_t)>;

		ComboBox(std::vector<std::string> options, std::string description, UpdateHandler handler)
				: m_options(std::move(options))
				, m_currentSelection(m_options.begin())
				, m_description(std::move(description))
				, m_onChangeHandler(std::move(handler)) {}

		std::vector<std::string> m_options;
		Selection m_currentSelection;
		std::string m_description;
		UpdateHandler m_onChangeHandler;
	};
}
