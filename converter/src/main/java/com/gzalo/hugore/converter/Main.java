package com.gzalo.hugore.converter;

import org.apache.commons.io.FilenameUtils;
import picocli.CommandLine;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.concurrent.Callable;
import java.util.stream.Stream;

@CommandLine.Command(name = "convert", mixinStandardHelpOptions = true, description = "Converts a BigFile folder into standard formats")
public class Main implements Callable<Integer> {
    @CommandLine.Parameters(index = "0", description = "Input path")
    private Path inputRoot;

    @CommandLine.Parameters(index = "1", description = "Output path")
    private Path outputRoot;

    @Override
    public Integer call() throws IOException {
        if (!inputRoot.toFile().isDirectory()) {
            throw new IllegalArgumentException("Input must be a directory");
        }

        try(Stream<Path> inputs = Files.walk(inputRoot)
                .filter(Files::isRegularFile)){
            inputs.forEach(input -> {
                final String extension = FilenameUtils.getExtension(input.getFileName().toString());
                final FileType type = FileType.valueOf(extension.toUpperCase());
                final Path output = outputRoot.resolve(inputRoot.relativize(input));
                try {
                    if(type.getConverter().convert(input, output) != 0){
                        System.out.println("Failed to convert " + input + " to " + output);
                    }
                } catch (IOException e) {
                    System.out.println("I/O exception processing " + input);
                }
            });
        }

        return 0;
    }

    public static void main(String[] args) {
        int exitCode = new CommandLine(new Main()).execute(args);
        System.exit(exitCode);
    }
}