#include "typing.h"
#include <imgui.h>
#include <array>
#include "util.h"

static std::string_view get_word(typing_state& state)
{
    std::uniform_int_distribution<int> dist(0, std::size(state.possible_words) - 1);
    auto index = dist(state.rng);
    return state.possible_words[index];
}

static int get_wrap_index(typing_state& state, int word_start_index)
{
    auto get_width = [&](const std::string_view& str){
        auto size = ImGui::CalcTextSize(str.data(), str.data() + str.size());
        return size.x;
    };

    const auto space_width = get_width(" ");

    constexpr auto max_width = 500;
    auto line_width = get_width(state.word_buffer[word_start_index]);

    int index = (word_start_index + 1) % state.num_words;
    while (line_width <= max_width)
    {
        line_width += space_width;
        line_width += get_width(state.word_buffer[index]);
        index = (index + 1) % state.num_words;
    }

    return index;
}

static void calc_wrap_indices(typing_state& state)
{
    for (int i = 0; i < 2; ++i)
    {
        state.word_buffer_wrap_indices[i + 1] = get_wrap_index(state, state.word_buffer_wrap_indices[i]);
    }
}

static void start_typing_test(typing_state& state)
{
    state.test_started = true;
    state.test_start_time = get_time();
    state.characters_typed = 0;
    state.word_buffer_head_index = 0;

    for (auto&& word : state.word_buffer)
    {
        word = get_word(state);
    }

    for (auto&& value : state.error_words)
    {
        value = false;
    }

    for (auto&& index : state.word_buffer_wrap_indices)
    {
        index = 0;
    }

    if (state.text_render_method == trm_10fastfingers)
    {
        calc_wrap_indices(state);
    }
}

static void end_typing_test(typing_state& state)
{
    state.test_started = false;
    state.word_buffer_head_index = 0;
    for (auto&& word : state.word_buffer)
    {
        word = "";
    }
    auto length_multiplier = 60 / state.test_length_in_seconds.count();
    state.last_wpm = static_cast<float>(state.characters_typed) / 5 * length_multiplier;
}

typing_state init_typing_state()
{
    auto seed = std::random_device{}();
    typing_state state{std::mt19937{seed}};

    state.possible_words = {"about", "above", "add", "after", "again", "air", "all", "almost", "along", "also", "always", "America", "an", "and", "animal", "another", "answer", "any", "are", "around", "as", "ask", "at", "away", "back", "be", "because", "been", "before", "began", "begin", "being", "below", "between", "big", "book", "both", "boy", "but", "by", "call", "came", "can", "car", "carry", "change", "children", "city", "close", "come", "could", "country", "cut", "day", "did", "different", "do", "does", "don't", "down", "each", "earth", "eat", "end", "enough", "even", "every", "example", "eye", "face", "family", "far", "father", "feet", "few", "find", "first", "follow", "food", "for", "form", "found", "four", "from", "get", "girl", "give", "go", "good", "got", "great", "group", "grow", "had", "hand", "hard", "has", "have", "he", "head", "hear", "help", "her", "here", "high", "him", "his", "home", "house", "how", "idea", "if", "important", "in", "Indian", "into", "is", "it", "its", "it's", "just", "keep", "kind", "know", "land", "large", "last", "later", "learn", "leave", "left", "let", "letter", "life", "light", "like", "line", "list", "little", "live", "long", "look", "made", "make", "man", "many", "may", "me", "mean", "men", "might", "mile", "miss", "more", "most", "mother", "mountain", "move", "much", "must", "my", "name", "near", "need", "never", "new", "next", "night", "no", "not", "now", "number", "of", "off", "often", "oil", "old", "on", "once", "one", "only", "open", "or", "other", "our", "out", "over", "own", "page", "paper", "part", "people", "picture", "place", "plant", "play", "point", "put", "question", "quick", "quickly", "quite", "read", "really", "right", "river", "run", "said", "same", "saw", "say", "school", "sea", "second", "see", "seem", "sentence", "set", "she", "should", "show", "side", "small", "so", "some", "something", "sometimes", "song", "soon", "sound", "spell", "start", "state", "still", "stop", "story", "study", "such", "take", "talk", "tell", "than", "that", "the", "their", "them", "then", "there", "these", "they", "thing", "think", "this", "those", "thought", "three", "through", "time", "to", "together", "too", "took", "tree", "try", "turn", "two", "under", "until", "up", "us", "use", "very", "walk", "want", "was", "watch", "water", "way", "we", "well", "went", "were", "what", "when", "where", "which", "while", "white", "who", "why", "will", "with", "without", "word", "work", "world", "would", "write", "year", "you", "young", "your"};
    state.test_started = false;
    state.input_buffer[0] = 0;

    ImGui::GetIO().FontGlobalScale = 1.25f;

    return state;
}

static int input_text_callback(ImGuiTextEditCallbackData* data)
{
    auto&& state = *static_cast<typing_state*>(data->UserData);

    auto input = std::string_view{data->Buf};

    auto clear_input_buffer = [&]
    {
        data->DeleteChars(0, data->BufTextLen);
    };

    if (!state.test_started)
    {
        clear_input_buffer();
        return 0;
    }

    // In ehm_continue, ignore superfluous spaces.
    if (state.error_handling_method == ehm_continue && input == " ")
    {
        clear_input_buffer();
        return 0;
    }

    // We accept a word when the typist has entered a space after it.
    if (input.back() == ' ')
    {
        bool is_right_word = input.substr(0, input.size() - 1) == state.current_word();
        if (is_right_word)
        {
            state.characters_typed += input.size();
        }

        if (is_right_word || state.error_handling_method == ehm_continue)
        {
            auto old_head = state.word_buffer_head_index;
            state.word_buffer_head_index = (state.word_buffer_head_index + 1) % state.num_words;
            clear_input_buffer();

            switch (state.text_render_method)
            {
                case trm_single_word_top_down:
                    state.word_buffer[old_head] = get_word(state);
                    break;

                case trm_10fastfingers:
                    if (state.word_buffer_head_index == state.word_buffer_wrap_indices[1])
                    {
                        // Move down one line and fill the completed line with new words.
                        for (int i : circular_range(state,
                                    state.word_buffer_wrap_indices[0],
                                    state.word_buffer_wrap_indices[1]))
                        {
                            state.word_buffer[i] = get_word(state);
                        }
                        state.word_buffer_wrap_indices[0] = state.word_buffer_wrap_indices[1];
                        calc_wrap_indices(state);
                    }
                    break;
            }

            state.error_words[old_head] = !is_right_word;
            state.error_index = -1;
        }
    }
    else
    {
        auto get_error_index = [&]
        {
            for (int i = 0; i < static_cast<int>(input.size()); ++i)
            {
                if (input[i] != state.current_word()[i])
                {
                    return i;
                }
            }

            return -1;
        };

        state.error_index = get_error_index();
    }

    return 0;
}

static void draw_settings_window(typing_state& state)
{
    ImGui::Begin("Settings", &state.show_settings_window);

    ImGui::InputFloat("Font global scale", &ImGui::GetIO().FontGlobalScale);

    const char* trm_items[] = {
        "Single word, top-down",
        "10fastfingers",
    };

    ImGui::ListBox("Text render method", &state.text_render_method, trm_items,
            std::size(trm_items), 3);

    const char* edm_items[] = {
        "Whole word",
        "From start of error",
        "None",
    };

    ImGui::ListBox("Error display method", &state.error_display_method, edm_items,
            std::size(edm_items), 3);

    const char* ehm_items[] = {
        "Continue",
        "Stop",
    };

    ImGui::ListBox("Error handling method", &state.error_handling_method, ehm_items,
            std::size(ehm_items), 3);

    ImGui::End();
}

namespace colors
{
    const ImVec4 red = {1, 0, 0, 1}, white = {1, 1, 1, 1}, green = {0, 1, 0, 1};
}

static void draw_split_error_word(typing_state& state, const std::string_view& word)
{
    ImGui::TextColored(colors::white, "%.*s", state.error_index, word.data());
    ImGui::SameLine(0, 0);
    auto error_start = word.data() + state.error_index;
    auto error_size = static_cast<int>(word.size()) - (state.error_index - 1);
    ImGui::TextColored(colors::red, "%.*s", error_size, error_start);
}

static bool check_error_word(typing_state& state, int index, ImVec4& color, const std::string_view& word)
{
    if (index == state.word_buffer_head_index && state.error_display_method != edm_none && state.error_index != -1)
    {
        if (state.error_display_method == edm_from_start_of_error)
        {
            draw_split_error_word(state, word);
            return true;
        }

        if (state.error_display_method == edm_whole_word)
        {
            color = colors::red;
        }
    }

    return false;
}
    
static void draw_words_single_word_top_down(typing_state& state)
{
    ImGui::InputText("", state.input_buffer, std::size(state.input_buffer),
            ImGuiInputTextFlags_CallbackAlways, input_text_callback,
            &state);

    constexpr auto num_words_visible = 10;

    for (int i = 0; i < num_words_visible; ++i)
    {
        auto index = (i + state.word_buffer_head_index) % state.num_words;
        auto&& word = state.word_buffer[index];
        if (!word.data())
        {
            break;
        }

        auto color = colors::white;

        if (check_error_word(state, index, color, word))
        {
            continue;
        }

        // To avoid format specifiers in the word being parsed by ImGui::Text, we need to
        // pass the word as an argument to %s.
        // To support non-zero-terminated strings, we use %.*s, which allows us to specify how
        // many characters to display from the string, instead of relying on a zero at the end.
        ImGui::TextColored(color, "%.*s", word.size(), word.data());
    }
}

static void draw_words_10fastfingers(typing_state& state)
{
    int i = state.word_buffer_wrap_indices[0];
    int end = state.word_buffer_wrap_indices[2];
    bool newline = true;
    for (; i < end; ++i)
    {
        auto index = i % state.num_words;
        auto&& word = state.word_buffer[index];
        if (!word.data())
            break;

        if (!newline)
        {
            ImGui::Text(" ");
            ImGui::SameLine(0, 0);
        }

        auto color = colors::white;

        if (index < state.word_buffer_head_index ||
                index < state.word_buffer_wrap_indices[0])
        {
            if (state.error_words[index])
            {
                color = colors::red;
            }
            else
            {
                color = colors::green;
            }
        }

        if (!check_error_word(state, index, color, word))
        {
            ImGui::TextColored(color, "%.*s", word.size(), word.data());
        }

        newline = index == state.word_buffer_wrap_indices[1] - 1 ||
                index == end - 1;

        if (!newline)
        {
            ImGui::SameLine(0, 0);
        }
    }

    ImGui::InputText("", state.input_buffer, std::size(state.input_buffer),
            ImGuiInputTextFlags_CallbackAlways, input_text_callback,
            &state);
}

static void draw_words(typing_state& state)
{
    using func_type = void(*)(typing_state&);
    func_type funcs[trm_max] = {
        draw_words_single_word_top_down,
        draw_words_10fastfingers,
    };

    funcs[state.text_render_method](state);
}

void draw_typing_window(typing_state& state)
{
    if (ImGui::Button("Start"))
    {
        start_typing_test(state);
    }

    ImGui::SameLine();

    if (ImGui::Button("Settings"))
    {
        state.show_settings_window = !state.show_settings_window;
    }

    if (state.show_settings_window)
    {
        draw_settings_window(state);
    }

    if (state.test_started)
    {
        auto current_time = get_time();
        auto elapsed_time = current_time - state.test_start_time;
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<float>>(elapsed_time);
        auto seconds_left = state.test_length_in_seconds - elapsed_seconds;
        if (seconds_left < 0s)
        {
            end_typing_test(state);
        }
        else
        {
            ImGui::Text("%.1f", seconds_left.count());
        }

        draw_words(state);
    }
    else
    {
        ImGui::Text("WPM: %.2f", state.last_wpm);
    }
}
