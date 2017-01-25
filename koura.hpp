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

    class entity {
    public:
        enum class type {
            text, number, object, sequence
        };

        entity() = default;
        entity(const entity&) = default;
        entity(entity&&) = default;
        entity& operator=(const entity&) = default;
        entity& operator=(entity&&) = default;
        ~entity() = default;

        entity (text_t value) : m_type{type::text}, m_value{std::move(value)} {}
        entity (number_t value) : m_type{type::number}, m_value{std::move(value)} {}
        entity (object_t value) : m_type{type::object}, m_value{std::move(value)} {}
        entity (sequence_t value) : m_type{type::sequence}, m_value{std::move(value)} {}

        auto get_type() -> type { return m_type; }

        template <class T>
        auto get_value() -> T& { return std::any_cast<T&>(m_value); }

    private:
        type m_type;
        std::any m_value;
    };

    class context {
    public:
        template <class T>
        void add_entity (std::string_view key, T&& value) {
            m_entities.emplace(key, entity{std::forward<T>(value)});
        }

        auto get_entity(const std::string& key) -> entity& { return m_entities.at(key); }

    private:
        std::unordered_map<std::string, entity> m_entities;
    };

    namespace filters {
        inline std::string capitalize (std::string_view text, context&) {
            std::string ret {text};
            for (auto&& c : ret) {
                c = std::toupper(c);
            }
            return ret;
        }
    }


    class engine;

    namespace detail {
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

        inline entity& parse_named_entity (std::istream& in, context& ctx) {
            auto name = get_identifier(in);

            auto& ent = ctx.get_entity(std::string{name});

            eat_whitespace(in);

            if (ent.get_type() == entity::type::text) {
                return ent;
            }
            else if (ent.get_type() == entity::type::object) {
                auto next = in.peek();

                if (next == '.') {
                    in.get();
                    if (ent.get_type() != entity::type::object) {
                        //TODO error
                    }

                    auto field_name = get_identifier(in);
                    auto& ent_obj = ent.get_value<object_t>();

                    if (!ent_obj.count(field_name)) {
                        //TODO error
                    }

                    return ent_obj.at(field_name);
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

            //TODO error
        }


        inline void expect_text(std::istream& in, std::string_view expected) {
            std::string got;
            in >> got;
            if (got != expected) {
                //TODO error
            }
        }

        inline void eat_tag (std::istream& in) {
            eat_whitespace(in);

            if (in.get() != '{') {
                //TODO error
            }

            if (in.get() != '%') {
                //TODO error
            }

            while (in.get() != '%') {
            }

            if (in.get() != '}') {
                //TODO error
            }
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
                //TODO error
            }

            auto start_pos = in.tellg();

            if (ent.get_type() != entity::type::sequence) {
                //TODO error
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

            if (cond) {
                process_until_tag(eng,in,out,ctx,"endif");
            }
            else {
                skip_until_tag(eng,in,out,ctx,"endif");
            }
            eat_tag(in);
        }
    }

    class engine {
    public:
        using expression_handler_t = std::function<void(engine&,std::istream&, std::ostream&, context&)>;
        using filter_t = std::function<std::string(std::string_view, context&)>;

        engine() :
            m_expression_handlers{
              {"if", detail::handle_if_expression},
              {"unless", detail::handle_unless_expression},
              {"set", detail::handle_set_expression},
              {"for", detail::handle_for_expression}
            },

            m_filters{
              {"capitalize", filters::capitalize}
            }
        {}

        void render (std::istream& in, std::ostream& out, context& ctx) {
            while (in) {
                detail::stream_up_to_tag(in, out);
                if (in) {
                    detail::process_tag(*this, in, out, ctx);
                }
            }
        }

        void register_custom_expression (std::string_view name, expression_handler_t handler) {
            m_expression_handlers.emplace(std::string{name}, handler);
        }

        void register_custom_filter (std::string_view name, filter_t filter) {
            m_filters.emplace(std::string{name}, filter);
        }



        inline void handle_variable_tag (std::istream& in, std::ostream& out, context& ctx) {
            auto ent = detail::parse_named_entity(in, ctx);
            if (ent.get_type() != entity::type::text) {
                //TODO error
            }

            auto text = ent.get_value<text_t>();

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
                //TODO error
            }

            m_expression_handlers[tag_name](*this,in,out,ctx);
        }

        auto handle_filter (std::istream& in, context& ctx, std::string_view text) -> std::string {
            auto filter_name = detail::get_identifier(in);

            if (!m_filters.count(filter_name)) {
                //TODO error
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
                //Get rid of one trailing whitespace
                if (in.peek() == '\n') in.get();
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
                            if (in.peek() == '}') {
                                return;
                            }
                            in.get();
                        }
                    }();
                }
            }

            //Get rid of one trailing whitespace
            if (in.peek() == '\n') in.get();
        }
    }
}
