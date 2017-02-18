#ifndef KOURA_HPP
#define KOURA_HPP

#include <iostream>
#include <optional>
#include <cctype>
#include <functional>
#include <cassert>
#include <utility>
#include <unordered_map>
#include <string_view>
#include <any>
#include <algorithm>

namespace koura {

    class entity;

    using number_t = int;
    using text_t = std::string;
    using object_t = std::unordered_map<std::string, entity>;
    using sequence_t = std::vector<entity>;

    /// An error which occured during rendering
    class render_error : public std::runtime_error {
    public:
        render_error(std::istream& in) :
            std::runtime_error{"Render error occurred"}
        {}
    };

    /// An entity within the Koura templating language.
    ///
    /// Can be text, a number, an object (associative array) or sequence.
    class entity {
    public:
        /// Used to distinguish the type of an entity.
        enum class type {
            text, number, object, sequence
        };

        entity () = default;
        entity (text_t value) : m_type{type::text}, m_value{std::move(value)} {}
        entity (number_t value) : m_type{type::number}, m_value{std::move(value)} {}
        entity (object_t value) : m_type{type::object}, m_value{std::move(value)} {}
        entity (sequence_t value) : m_type{type::sequence}, m_value{std::move(value)} {}

        /// Get the type of this entity.
        auto get_type() -> type { return m_type; }

        /// Get the value of the entity as the given type.
        /// \requires `T` is one of `number_t`, `text_t`, `object_t` or `sequence_t`.
        /// \throws `koura::wrong_entity_type` if this entity does not store a `T`.
        template <class T>
        auto get_value() -> T& { return std::any_cast<T&>(m_value); }

    private:
        type m_type;
        std::any m_value;
    };


    /// Manages all of the Koura variables.
    class context {
    public:
        /// Add an entity to the context with the key `key` and the value `value` to the context.
        template <class T>
        void add_entity (std::string_view key, T&& value) {
            m_entities.emplace(key, entity{std::forward<T>(value)});
        }

        /// Gets a reference to the entity with the given key.
        /// \throws `std::out_of_range` if there is no entity matching `key`.
        auto get_entity(const std::string& key) -> entity& { return m_entities.at(key); }


        /// Return whether or not an entity with the given name exists
        bool contains(const std::string& key) { return m_entities.count(key); }

    private:
        std::unordered_map<std::string, entity> m_entities;
    };

    /// All of the standard Koura text filters.
    namespace filters {
        /// Capitalises the given text
        inline std::string capitalise (std::string_view text, context&) {
            std::string ret {text};
            for (auto&& c : ret) {
                c = std::toupper(c);
            }
            return ret;
        }
    }


    class engine;

    namespace detail {
        inline void eat_single_trailing_whitespace (std::istream& in) {
            if (in.peek() == '\n') in.get();
        }

        inline bool is_block_tag (std::string_view tag) {
            return (tag == "if" || tag == "for");
        }

        inline void stream_up_to_tag (std::istream& in, std::ostream& out) {
            char c;
            while (c = in.get(), in) {
                if (c == '{') {
                    auto next = in.peek();
                    if (next == '{' || next == '%') {
                        in.unget();
                        return;
                    }
                }
                out << c;
            }
        }

        inline void skip_up_to_tag (std::istream& in, std::ostream& out) {
            char c;
            while (c = in.get(), in) {
                if (c == '{') {
                    auto next = in.peek();
                    if (next == '{' || next == '%') {
                        in.unget();
                        return;
                    }
                }
            }
        }

        inline void eat_whitespace (std::istream& in) {
            while (std::isspace(in.peek())) {
                in.get();
            }
        }

        inline char peek (std::istream& in) {
            eat_whitespace(in);
            return in.peek();
        }

        inline std::string get_identifier (std::istream& in) {
            eat_whitespace(in);
            std::string name = "";
            while (std::isalpha(in.peek()) || in.peek() == '_') {
                name += in.get();
            }

            return name;
        }

        inline entity& parse_nested_object (std::istream& in, entity& ent) {
            if (ent.get_type() != entity::type::object) {
                throw render_error{in};
            }

            auto field_name = get_identifier(in);
            auto& ent_obj = ent.get_value<object_t>();

            if (!ent_obj.count(field_name)) {
                throw render_error{in};
            }

            auto& nested = ent_obj.at(field_name);

            //We need to go deeper
            if (in.peek() == '.') {
                in.get();
                auto& deeper = parse_nested_object(in, nested);
                return deeper;
            }

            return nested;
        }

        inline entity& parse_named_entity (std::istream& in, context& ctx) {
            auto name = get_identifier(in);

            auto& ent = ctx.get_entity(std::string{name});

            eat_whitespace(in);

            if (ent.get_type() == entity::type::text || ent.get_type() == entity::type::number) {
                return ent;
            }
            else if (ent.get_type() == entity::type::object) {
                auto next = in.peek();

                if (next == '.') {
                    in.get();
                    return parse_nested_object(in, ent);
                }
            }
            else if (ent.get_type() == entity::type::sequence) {
                return ent;
            }
        }

        inline koura::entity parse_entity (std::istream& in, koura::context& ctx) {
            auto c = peek(in);
            //String literal
            if (c == '\'') {
                in.get();
                koura::text_t text;
                while (in.peek() != '\'') {
                    text += in.get();
                }
                in.get();
                return entity{text};
            }
            //Number literal
            else if (std::isdigit(c)) {
                koura::number_t num;
                in >> num;
                return entity{num};
            }
            //List literal
            else if (c == '[') {
                //TODO
            }
            //Named entity
            else {
                return parse_named_entity(in,ctx);
            }

            throw render_error{in};
        }


        inline void expect_text(std::istream& in, std::string_view expected) {
            std::string got;
            in >> got;
            if (got != expected) {
                throw render_error{in};
            }
        }

        inline void eat_tag (std::istream& in) {
            eat_whitespace(in);

            if (in.get() != '{') {
                throw render_error{in};
            }

            if (in.get() != '%') {
                throw render_error{in};
            }

            while (in.get() != '%') {
            }

            if (in.get() != '}') {
                throw render_error{in};
            }

            eat_single_trailing_whitespace(in);
        }

        inline bool is_truthy (entity& ent) {
            return true;
        }

        inline bool is_next_tag (std::istream& in, std::string_view tag) {
            auto start_pos = in.tellg();
            auto reset = [start_pos,&in] {in.seekg(start_pos);};

            if (in.get() != '{') {
                reset();
                return false;
            }

            if (in.get() != '%') {
                reset();
                return false;
            }

            while (std::isspace(in.peek())) {
                in.get();
            }

            auto id = get_identifier(in);

            if (id == tag) {
                reset();
                return true;
            }

            reset();
            return false;
        }

        void process_tag (engine& eng, std::istream& in, std::ostream& out, context& ctx);
        void skip_tag (engine& eng, std::istream& in, std::ostream& out, context& ctx);

        inline void process_until_tag (engine& eng, std::istream& in, std::ostream& out, context& ctx, std::string_view tag) {
            while (true) {
                stream_up_to_tag(in,out);
                if (is_next_tag(in,tag)) {
                    return;
                }
                process_tag(eng,in,out,ctx);
            }
        }

        inline void skip_until_tag (engine& eng, std::istream& in, std::ostream& out, context& ctx, std::string_view tag) {
            while (true) {
                skip_up_to_tag(in,out);
                if (is_next_tag(in,tag)) {
                    return;
                }
                skip_tag(eng,in,out,ctx);
            }
        }


        inline void handle_for_expression (engine& eng, std::istream& in, std::ostream& out, context& ctx) {
            auto loop_var_id = get_identifier(in);
            expect_text(in, "in");
            auto ent = parse_entity(in,ctx);

            if (in.get() != '%' || in.get() != '}') {
                throw render_error{in};
            }

            auto start_pos = in.tellg();

            if (ent.get_type() != entity::type::sequence) {
                throw render_error{in};
            }

            auto seq = ent.get_value<sequence_t>();

            for (auto&& loop_var : seq) {
                in.seekg(start_pos);
                auto ctx_with_loop_var = ctx;
                ctx_with_loop_var.add_entity(loop_var_id, loop_var);

                process_until_tag(eng,in,out,ctx_with_loop_var,"endfor");
            }

            eat_tag(in);
        }

        inline void handle_unless_expression (engine& eng, std::istream& in, std::ostream& out, context& ctx) {

        }

        inline void handle_set_expression (engine& eng, std::istream& in, std::ostream& out, context& ctx) {
            auto& ent = parse_named_entity(in, ctx);
            auto val = parse_entity(in, ctx);

            switch (ent.get_type()) {
            case koura::entity::type::number:
                ent.get_value<koura::number_t>() = val.get_value<koura::number_t>();
                break;
            case koura::entity::type::text:
            {
                ent.get_value<koura::text_t>() = val.get_value<koura::text_t>();
                auto a = ent.get_value<koura::text_t>();
                auto b = val.get_value<koura::text_t>();
                break;
            }
            case koura::entity::type::object:
                ent.get_value<koura::object_t>() = val.get_value<koura::object_t>();
                break;
            case koura::entity::type::sequence:
                //TODO
                break;
            }

            eat_whitespace(in);
            assert(in.get() == '%');
            assert(in.get() == '}');
        }

        inline void handle_if_expression (engine& eng, std::istream& in, std::ostream& out, context& ctx) {
            bool cond = false;
            try {
                auto ent = parse_entity(in, ctx);
                cond = is_truthy(ent);
            }
            catch (std::out_of_range& e) {
                cond = false;
            }

            eat_whitespace(in);
            assert(in.get() == '%');
            assert(in.get() == '}');
            eat_single_trailing_whitespace(in);

            if (cond) {
                while (true) {
                    stream_up_to_tag(in,out);
                    if (is_next_tag(in, "elseif") || is_next_tag(in, "else")) {
                        skip_until_tag(eng,in,out,ctx,"endif");
                    }
                    else if (is_next_tag(in, "endif")) {
                        break;
                    }
                    else {
                        process_tag(eng,in,out,ctx);
                    }
                }
            }
            else {
                while (true) {
                    skip_up_to_tag(in,out);

                    //TODO ifelse

                    if (is_next_tag(in, "else")) {
                        eat_tag(in);
                        while (true) {
                            stream_up_to_tag(in,out);
                            if (is_next_tag(in, "endif")) {
                                break;
                            }
                            else {
                                process_tag(eng,in,out,ctx);
                            }
                        }
                        break;
                    }
                    else if (is_next_tag(in, "endif")) {
                        break;
                    }
                    else {
                        skip_tag(eng,in,out,ctx);
                    }
                }
            }
            eat_tag(in);
        }
    }

    /// The Koura rendering engine.
    class engine {
    public:
        /// The type of a custom expression handler.
        using expression_handler_t = std::function<void(engine&,std::istream&, std::ostream&, context&)>;

        /// The type of a custom text filter.
        using filter_t = std::function<std::string(std::string_view, context&)>;

        engine() :
            m_expression_handlers{
              {"if", detail::handle_if_expression},
              {"unless", detail::handle_unless_expression},
              {"set", detail::handle_set_expression},
              {"for", detail::handle_for_expression}
            },

            m_filters{
              {"capitalise", filters::capitalise}
            }
        {}


        /// Render the text from `in` to `out` using the context `ctx`.
        void render (std::istream& in, std::ostream& out, context& ctx) {
            while (in) {
                detail::stream_up_to_tag(in, out);
                if (in) {
                    detail::process_tag(*this, in, out, ctx);
                }
            }
        }


        /// Register a custom expression handler.
        void register_custom_expression (std::string_view name, expression_handler_t handler) {
            m_expression_handlers.emplace(std::string{name}, handler);
        }

        /// Register a custom text filter.
        /// After a filter is registered, it can be used just like a normal filter
        /// E.g. `eng.register_custom_filter("upcase_even", upcase_even);` `{{thing | upcase_even}}`
        void register_custom_filter (std::string_view name, filter_t filter) {
            m_filters.emplace(std::string{name}, filter);
        }


        inline void handle_variable_tag (std::istream& in, std::ostream& out, context& ctx) {
            auto ent = detail::parse_named_entity(in, ctx);
            if (ent.get_type() != entity::type::text && ent.get_type() != entity::type::number) {
                throw render_error{in};
            }

            auto text = ent.get_type() == entity::type::text ? ent.get_value<text_t>() : std::to_string(ent.get_value<number_t>());

            detail::eat_whitespace(in);

            while (in.peek() == '|') {
                in.get();
                text = handle_filter(in, ctx, text);
                detail::eat_whitespace(in);
            }

            out << text;

            assert(in.get() == '}');
            assert(in.get() == '}');
        }

        void handle_expression_tag (std::istream& in, std::ostream& out, context& ctx) {
            std::string tag_name;
            in >> tag_name;

            if (!m_expression_handlers.count(tag_name)) {
                throw render_error{in};
            }

            m_expression_handlers[tag_name](*this,in,out,ctx);
        }

        auto handle_filter (std::istream& in, context& ctx, std::string_view text) -> std::string {
            auto filter_name = detail::get_identifier(in);

            if (!m_filters.count(filter_name)) {
                throw render_error{in};
            }

            return m_filters[filter_name](text,ctx);
        }

    private:
        std::unordered_map<std::string, expression_handler_t> m_expression_handlers;
        std::unordered_map<std::string, filter_t> m_filters;
    };

    namespace detail {
        inline void process_tag (engine& eng, std::istream& in, std::ostream& out, context& ctx) {
            assert(in.get() == '{');

            auto next = in.get();

            if (next == '{') {
                eat_whitespace(in);
                eng.handle_variable_tag(in,out,ctx);
            }

            if (next == '%') {
                eat_whitespace(in);
                eng.handle_expression_tag(in,out,ctx);
                eat_single_trailing_whitespace(in);
            }
        }

        inline void skip_tag (engine& eng, std::istream& in, std::ostream& out, context& ctx) {
            assert(in.get() == '{');

            auto next = in.get();

            if (next == '{') {
                [&]{
                    while (true) {
                        while (in.peek() != '}') {
                            in.get();
                        }
                        if (in.peek() == '}') {
                            return;
                        }
                        in.get();
                    }
                }();
            }

            else if (next == '%') {
                eat_whitespace(in);
                auto id = get_identifier(in);
                if (is_block_tag(id)) {
                    skip_until_tag(eng,in,out,ctx,std::string{"end"} + id);
                    eat_tag(in);
                }
                else {
                    [&]{
                        while (true) {
                            while (in.peek() != '%') {
                                in.get();
                            }
                            in.get();
                            if (in.peek() == '}') {
                                eat_single_trailing_whitespace(in);
                                return;
                            }
                            in.get();
                        }
                    }();
                }
            }
        }
    }
}

#endif
