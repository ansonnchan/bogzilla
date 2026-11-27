# Data notices

## enable1.txt

The ENABLE word list (Enhanced North American Benchmark Lexicon), public domain. Copy from
[dolph/dictionary](https://github.com/dolph/dictionary). Used as the fallback dictionary for
sites that don't expose their own answers.

## frequency.txt

Derived from [FrequencyWords](https://github.com/hermitdave/FrequencyWords) by Hermit Dave
(`content/2018/en/en_50k.txt`, built from [OpenSubtitles 2018](http://opus.nlpl.eu/OpenSubtitles2018.php)),
licensed under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/).

Changes: kept only a-z words of 3+ letters, dropped the counts, one word per line in rank
order. This file is shared under the same CC BY-SA 4.0 licence. Regenerate it with
`node scripts/fetch-frequency.mjs`.
