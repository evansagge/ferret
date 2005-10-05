module Ferret::Analysis
  # An Analyzer builds TokenStreams, which analyze text.  It thus represents
  # a policy for extracting index terms from text.
  #
  # Typical implementations first build a Tokenizer, which breaks the stream
  # of characters from the Reader into raw Tokens. One or more TokenFilter s
  # may then be applied to the output of the Tokenizer.
  #
  # The default Analyzer just creates a LowerCaseTokenizer which converts
  # all text to lowercase tokens. See LowerCaseTokenizer for more details.
  class Analyzer
    # Creates a TokenStream which tokenizes all the text in the provided
    # Reader. Override to allow Analyzer to choose strategy based on
    # document and/or field.
    # string:: the string representing the text in the field
    # field_name:: name of the field. Not required.
    def token_stream(field_name, string)
      return LowerCaseTokenizer.new(string)
    end
  end

  # An Analyzer that uses WhiteSpaceTokenizer.
  class WhiteSpaceAnalyzer < Analyzer
    def token_stream(field_name, string)
      return WhiteSpaceTokenizer.new(string)
    end
  end

  # Filters LetterTokenizer with LowerCaseFilter and StopFilter.
  class StopAnalyzer < Analyzer

    # An array containing some common English words that are not usually useful
    # for searching.
    ENGLISH_STOP_WORDS = [
      "a", "an", "and", "are", "as", "at", "be", "but", "by",
      "for", "if", "in", "into", "is", "it",
      "no", "not", "of", "on", "or", "s", "such",
      "t", "that", "the", "their", "then", "there", "these",
      "they", "this", "to", "was", "will", "with"
    ]

    # Builds an analyzer which removes words in the provided array.
    def initialize(stop_words = ENGLISH_STOP_WORDS)
      @stop_words = stop_words
    end

    # Filters LowerCaseTokenizer with StopFilter.
    def token_stream(field_name, string)
      return StopFilter.new(LowerCaseTokenizer.new(string), @stop_words)
    end
  end

  # An Analyzer that filters LetterTokenizer with LowerCaseFilter.
  class StandardAnalyzer < StopAnalyzer
    def token_stream(field_name, string)
      return StopFilter.new(LowerCaseFilter.new(StandardTokenizer.new(string)), @stop_words)
    end
  end


  # This analyzer is used to facilitate scenarios where different
  # fields require different analysis techniques.  Use #add_analyzer
  # to add a non-default analyzer on a field name basis.
  # See tc_per_field_analyzer_wrapper for example usage.
  class PerFieldAnalyzerWrapper < Analyzer

    # Constructs with default analyzer.
    #
    # default_analyzer:: Any fields not specifically defined to use a
    #                    different analyzer will use the one provided here.
    def initialize(default_analyzer)
      @default_analyzer = default_analyzer
      @analyzers = {}
    end

    # Defines an analyzer to use for the specified field.
    #
    # field_name:: field name requiring a non-default analyzer.
    # analyzer:: non-default analyzer to use for field
    def add_analyzer(field_name, analyzer)
      @analyzers[field_name] = analyzer
    end

    def token_stream(field_name, string)
      analyzer = @analyzers[field_name]
      if (analyzer == nil)
        analyzer = @default_analyzer;
      end

      return analyzer.token_stream(field_name, string)
    end
  end
end
