#include "../src/protocols/IParser.cpp"
class FileITCHParser {
   public:
    explicit FileITCHParser(IParser& parser) noexcept : parser_{parser} {}

   private:
    IParser parser_;
};
