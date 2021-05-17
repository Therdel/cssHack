//
// Created by theo on 21.11.20.
//
#pragma once

#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include <boost/mpl/list.hpp>

#include <utility>

#include "Input.hpp"
#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX
#include "Log.hpp"

namespace sc = boost::statechart;
namespace mpl = boost::mpl;

namespace States {
    struct Injected;
    struct Inactive;
    struct Active;

    struct MainMenu;

    struct Ejecting;
}

namespace Events {
    struct ToggleActive : sc::event<ToggleActive>{};
    struct EjectByUser  : sc::event<EjectByUser> {};
}

struct MainSM : sc::state_machine<MainSM,
                                  States::Injected> /* init-state */ {
};

struct States::Injected : sc::state<States::Injected,
                                    MainSM,           /* super-state: state machine */
                                    States::Inactive> /* init-state*/                  {

    using reactions = sc::transition<Events::EjectByUser, States::Ejecting>;

    Injected(my_context ctx) : my_base(std::move(ctx))
    , input()
    , _toggleActiveHandler(_setupKeyHandler()) {
        // entry
        Log::log("Enter State::Injected");
    }

    // FIXME: Racy - could be called during or after state exit, as Input and SM are unsynchronized.
    //        Fix by wrapping all Input events in own events. Possibly in an SDL event for less events + handlers.
    auto activate(const SDL_KeyboardEvent &event) -> bool {
        if (event.repeat == 0 &&
            event.type == SDL_KEYDOWN) {
            context<MainSM>().process_event(Events::ToggleActive{});
        }
        return Input::StealEvent;
    }

    Input input;
private:
    ScopedKeyHandler _toggleActiveHandler;
    auto _setupKeyHandler() -> ScopedKeyHandler {
        return ScopedKeyHandler{context<Injected>().input,
                                key_inject,
                                [this](auto &&event) { return activate(event); }};
    }
};

struct States::Inactive : sc::state<States::Inactive,
                                    States::Injected> /* super-state */ {

    using reactions = sc::transition<Events::ToggleActive, States::Active>;

    Inactive(my_context ctx) : my_base(std::move(ctx)){
        // entry
        Log::log("Enter State::Inactive");
    }
};

struct States::Active : sc::state<States::Active,
                                  States::Injected> /* super-state */ {
    using reactions = sc::transition<Events::ToggleActive, States::Inactive>;

    Active(my_context ctx) : my_base(std::move(ctx)) {
        // entry
        Log::log("Enter State::Active");
    }
};

struct States::Ejecting : sc::simple_state<States::Ejecting,
                                           States::Injected> /* super-state */ {
    Ejecting() {
        // entry
        Log::log("Enter State::Ejecting");
    }
};

auto test() {
    MainSM hack;
    hack.initiate();

    hack.process_event(Events::ToggleActive{});
}
