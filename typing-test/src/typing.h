#pragma once
#include <random>
#include <string>
#include <string_view>
#include <chrono>
using namespace std::literals::chrono_literals;

enum text_render_methods
{
    trm_single_word_top_down,
    trm_10fastfingers,
    trm_max,
};

enum error_display_methods
{
    edm_whole_word,
    edm_from_start_of_error,
    edm_none,
    edm_max,
};

enum error_handling_methods
{
    ehm_continue,
    ehm_stop,
    ehm_max,
};

struct typing_state
{
    using clock = std::chrono::steady_clock;

    std::mt19937 rng;

    std::vector<std::string> possible_words;

    // word_buffer is a circular buffer of references into possible_words.
    // It contains words already selected for typing.
    int word_buffer_head_index = 0;
    static constexpr auto num_words = 200;
    std::string_view word_buffer[num_words];
    bool error_words[num_words] = {};

    std::string_view current_word() const { return word_buffer[word_buffer_head_index]; }

    char input_buffer[4096];

    bool test_started = false;
    std::chrono::time_point<clock> test_start_time;
    int characters_typed = 0;
    std::chrono::duration<float> test_length_in_seconds = 60s;

    float last_wpm = 0;

    bool show_settings_window = false;
    // These are ints instead of enums so that they have the right type for passing to
    // imgui functions.
    int text_render_method = trm_single_word_top_down;
    int error_display_method = edm_from_start_of_error;
    int error_handling_method = ehm_continue;

    // The character index in the current word at which the typist has made a mistake.
    // This is -1 if there has not been an error so far.
    int error_index = -1;

    // In trm_10fastfingers, this array is used to store the indices into word_buffer where there's
    // a line break due to wrapping.
    // The first index is the one before the first visible line. It stores this to konw where to
    // start rendering.
    // The indices point to the words just after the line break.
    int word_buffer_wrap_indices[3] = {};
};

typing_state init_typing_state();
void draw_typing_window(typing_state& state);
