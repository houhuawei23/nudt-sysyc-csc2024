package utils;

import cg.grader.CompileException;
import cg.grader.Constants.VerdictStatus;
import cg.grader.Environment;
import cg.grader.ICompiler;
import cg.grader.ShowResult;

import java.io.File;
import java.io.IOException;
import java.net.URI;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicReference;
import java.util.function.BinaryOperator;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * @Author : YML
 * @Description:
 * @Date created at 2020/5/14 15:15
 **/
public class Compiler implements ICompiler
{
    @Override
    public void compile(String sourcePath, String targetPath) throws Exception
    {
        FileUtil.assertDictionary(sourcePath);
        AtomicBoolean isCppProject = new AtomicBoolean(false);
        AtomicBoolean isJavaProject = new AtomicBoolean(false);
        AtomicBoolean isRustProject = new AtomicBoolean(false);
        AtomicReference<String> cargoTomlAbsolutePath = new AtomicReference<>();
        StringBuilder fileList = new StringBuilder();
        StringBuilder linkCmd = new StringBuilder();
        Set<String> linkedDic = new HashSet<>();
        StringBuilder javaFileList = new StringBuilder();
        FileUtil.searchFilePathByBFS(sourcePath, file -> {
            if(file.isDirectory()){
                return false;
            }
            String path = file.getAbsolutePath();
            String suffix = path.substring(path.lastIndexOf(".") + 1);
            switch (suffix){
                case "hpp":
                case "hh":
                case "H":
                case "hxx":
                    isCppProject.set(true);
                case "h":
                    String linkPath = file.getParent();
                    if(!linkedDic.contains(linkPath)){
                        linkCmd.append(" ").append("-I ").append(linkPath);
                        linkedDic.add(linkPath);
                    }
                    break;
                case "cpp":
                case "CPP":
                case "c++":
                case "cxx":
                case "C":
                case "cc":
                case "cp":
                    isCppProject.set(true);
                case "c":
                    fileList.append(" ").append(path);
                    break;
                case "java":
                    javaFileList.append(" ").append(path);
                    isJavaProject.set(true);
                    break;
                default:
                        break;
            }
            if (file.isFile() && file.getName().equals("Cargo.toml")) {
                isRustProject.set(true);
                cargoTomlAbsolutePath.set(file.getParent());
            }
            return false;
        });
        String compileCmd;
        String mvCmd="";
        if (isJavaProject.get()) {
//            String antlr = "/usr/share/java/antlr-runtime-4.8.jar";
            String thirdLibBase = "/coursegrader/dockerext/";
            Set<String> libs = Stream.of(Objects.requireNonNull(new File(thirdLibBase).listFiles()))
                    .map(File::getName)
                    .filter(name -> name.endsWith(".jar"))
                    .filter(name -> !name.equals("ARMKernel.jar"))
                    .map(name->thirdLibBase + name)
                    .collect(Collectors.toSet());
            String outFolder = Environment.getExecFolderPath();
            Path outClassesPath = Paths.get(outFolder, "classes");
            File outClass = outClassesPath.toFile();
            if (outClass.exists()) {
                Optional<Boolean> result = Files
                        .walk(outClassesPath)
                        .sorted(Comparator.reverseOrder())
                        .map(Path::toFile)
                        .map(File::delete)
                        .reduce((a, b)->a&&b);
                if (! result.orElse(false)) {
                    throw new CompileException(String.format("target path %s is already exists and can not be deleted.", outClassesPath));
                }
            }
            if (!outClass.mkdirs()) {
                throw new CompileException(String.format("can not create target directory %s", outClassesPath));
            }
            // 创建/root/run/classes完成
            compileCmd = String.format("javac -d %s -encoding utf-8 -cp .:%s -sourcepath %s %s",
                    outClassesPath.toString(), String.join(":", libs), sourcePath, javaFileList);
            Environment.setExecName(Environment.getExecName() + ".jar");
            Path targetJar = Paths.get(outFolder, Environment.getExecName());
            File targetJarFile = targetJar.toFile();
            if (targetJarFile.exists() && !targetJarFile.delete()) {
                throw new CompileException(String.format("target jar file %s is already exists and can not be delete.", targetJar));
            }

//            Path manifest = Paths.get(sourcePath, "MANIFEST.MF");
//            if (!manifest.toFile().exists()) {
//                throw new CompileException("MANIFEST.MF is not exists.");
//            }

            // 复制antlr库到/root/run/classes/antlr-runtime-4.8.jar
//            Path targetAntlr = outClassesPath.resolve("antlr-runtime-4.8.jar");
//            Files.copy(Path.of(antlr), targetAntlr);

            // 复制res目录
            Path res = Paths.get(sourcePath, "res");
            File resFile = res.toFile();
            if(resFile.exists() && resFile.isDirectory()) {
                for (File subFile: resFile.listFiles()) {
                    Path oldPath = Path.of(subFile.getAbsolutePath());
                    Path newPath = outClassesPath.resolve(subFile.getName());
                    if (subFile.isFile()) {
                        FileUtil.copy(oldPath, newPath);
                    } else {
                        FileUtil.copyFolder(oldPath, newPath);
                    }
                }
//                FileUtil.copyFolder(res, outClassesPath);
            }

            // 构建jar命令
            compileCmd = String.format(
                    "%s && cd %s && %s && jar --create --file %s --main-class Compiler -C %s .",
                    compileCmd, outClassesPath,
                    libs.stream().map(x->"jar xf " + x).collect(Collectors.joining(" && ")),
                    targetJar, outClassesPath);
//            compileCmd = String.format("%s && jar --create --file %s --manifest %s -C %s ."
//                    , compileCmd, targetJar, manifest, outClassesPath);
            Environment.setCodeType("java");

        }
        else if (isRustProject.get()) {

            compileCmd = "cd "+ cargoTomlAbsolutePath.get()+" && /root/.cargo/bin/cargo build --release --target-dir " + Environment.getExecFolderPath();
            mvCmd = "mv "+ Environment.getExecFolderPath()+"/release/compiler"+" "+Environment.getExecFolderPath();
            Environment.setExecName("compiler");
            Environment.setCodeType("rust");
        }else {
            exec("mkdir -p /extlibs");
            File libFile = new File("/coursegrader/dockerext/lib.tar.gz");
            if (libFile.exists()) {
                exec("tar xaf /coursegrader/dockerext/lib.tar.gz -C /extlibs");
            }
            compileCmd = String.format("%s %s %s -o %s", this.getCompileCmdHeader(isCppProject.get()),
                    fileList, linkCmd, targetPath);
            Environment.setCodeType(isCppProject.get() ? "c++" : "c");
        }
        ResultRender.putInfo(ResultRender.compileCommandKey, compileCmd);
        ResultRender.putInfo(ResultRender.compileCommandKey, mvCmd);
        try {
            ResultRender.putInfo(ResultRender.compilerLogKey, exec(compileCmd));
            ResultRender.putInfo(ResultRender.compilerLogKey, exec(mvCmd));
        } catch (Exception e) {
            throw new CompileException(e.getMessage());
        }
    }

    @SuppressWarnings("unchecked")
    private static List<String> exec(String cmd) throws IOException, InterruptedException
    {
        String[] fullCmd = {"/bin/bash", "-c", cmd};
        Process process = new ProcessBuilder(fullCmd).start();
        final String STDOUT = "STDOUT";
        final String STDERR = "STDERR";
        Map<String,Object> info = new ConcurrentHashMap<>();
        Thread thread1 = ExecUtil.readStream(process.getInputStream(),info,STDOUT);
        Thread thread2 = ExecUtil.readStream(process.getErrorStream(),info,STDERR);
        int status = process.waitFor();
        thread1.join();
        thread2.join();
        if (status != 0) {
            ShowResult.setVerdict(VerdictStatus.CE.getVerdictStatus());
        }
        List<String> result = (List<String>)info.get(STDOUT);
        result.addAll((List<String>)info.get(STDERR));
        return result;
    }

    private String getCompileCmdHeader(boolean isCppProject) throws CompileException
    {
        String compilerType = Environment.getConfig().getString("compiler");
        if(!"gcc".equals(compilerType) && !"clang".equals(compilerType)){
            throw new CompileException("Unsupported Compiler Type");
        }
        String ans;
        if(isCppProject){
            if ("gcc".equals(compilerType)){
                ans = "g++ -std=c++17 -O2 -L/extlibs -I/extlibs -lm -lantlr4-runtime";
            }else{
                ans = "clang++ -std=c++17 -O2 -lm -L/extlibs -I/extlibs -lantlr4-runtime";
            }
        }else{
            ans = compilerType + " -std=c11 -O2 -lm";
        }
        return ans;
    }
}
