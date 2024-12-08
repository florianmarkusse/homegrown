package project

import (
	"cmd/common"
	"cmd/common/configuration"
	"cmd/common/converter"
	"cmd/common/flags"
	"cmd/common/flags/environment"
	"flag"
	"fmt"
	"strings"
)

const PROJECTS_LONG_FLAG = "projects"
const PROJECTS_SHORT_FLAG = "p"

func DisplayProject() {
	flags.DisplayArgumentInput(PROJECTS_SHORT_FLAG, PROJECTS_LONG_FLAG, "Select specific project(s, comma-separated) to be built", converter.ArrayIntoPrintableString(ConfiguredProjects))
}

func ValidateAndConvertProjects(projectsToBuild string, selectedProjects *[]string) bool {
	*selectedProjects = strings.FieldsFunc(projectsToBuild, func(r rune) bool {
		return r == ','
	})

	for _, selectedProject := range *selectedProjects {
		var isValidProjectName = false
		for _, configuredProjects := range ConfiguredProjects {
			if selectedProject == configuredProjects {
				isValidProjectName = true
			}
		}
		if !isValidProjectName {
			return false
		}
	}

	return true
}

func DisplayProjectConfiguration(selectedProjects []string) {
	var projectsConfiguration string
	if len(selectedProjects) > 0 {
		projectsConfiguration = converter.ArrayIntoPrintableString(selectedProjects[:])
	} else {
		projectsConfiguration = converter.ArrayIntoPrintableString(ConfiguredProjects)
	}
	configuration.DisplayStringArgument(PROJECTS_LONG_FLAG, projectsConfiguration)
}

func AddProjectAsFlag(project *string) {
	flag.StringVar(project, PROJECTS_LONG_FLAG, *project, "")
	flag.StringVar(project, PROJECTS_SHORT_FLAG, *project, "")
}

type Project int64

type ProjectStructure struct {
	CCompiler   string
	Linker      string
	Folder      string
	CodeFolder  string
	Environment string
}

type CommonConfig struct {
	CCompiler string
	Linker    string
}

var ELF = CommonConfig{
	CCompiler: "clang-19",
	Linker:    "ld.lld-19",
}

var EFI = CommonConfig{
	CCompiler: "clang-19",
	Linker:    "lld-link-19",
}

// If you add a project add it here
const KERNEL = "kernel"
const INTEROPERATION = "interoperation"
const UEFI_IMAGE_CREATOR = "uefi-image-creator"
const UEFI = "uefi"
const IMAGE_BUILDER = "image-builder"
const SHARED = "shared"
const POSIX = "posix"
const PLATFORM_ABSTRACTION = "platform-abstraction"
const X86 = "x86"

// and here
var kernelFolder = common.REPO_PROJECTS + "/" + KERNEL + "/"
var interoperationFolder = common.REPO_PROJECTS + "/" + INTEROPERATION + "/"
var uefiImageCreatorFolder = common.REPO_PROJECTS + "/" + UEFI_IMAGE_CREATOR + "/"
var uefiFolder = common.REPO_PROJECTS + "/" + UEFI + "/"
var imageBuilderFolder = common.REPO_PROJECTS + "/" + IMAGE_BUILDER + "/"
var sharedFolder = common.REPO_PROJECTS + "/" + SHARED + "/"
var posixFolder = common.REPO_PROJECTS + "/" + POSIX + "/"
var platformAbstractionFolder = common.REPO_PROJECTS + "/" + PLATFORM_ABSTRACTION + "/"
var x86Folder = common.REPO_PROJECTS + "/" + X86 + "/"

// and here
var PROJECT_STRUCTURES = map[string]*ProjectStructure{
	KERNEL: &ProjectStructure{
		CCompiler:   ELF.CCompiler,
		Linker:      ELF.Linker,
		Folder:      kernelFolder,
		CodeFolder:  kernelFolder + "code",
		Environment: string(environment.Freestanding),
	},
	INTEROPERATION: &ProjectStructure{
		CCompiler: ELF.CCompiler,
		// Is object file so adding standard for build output path
		Linker:      ELF.Linker,
		Folder:      interoperationFolder,
		CodeFolder:  interoperationFolder + "code",
		Environment: string(environment.Freestanding),
	},
	UEFI_IMAGE_CREATOR: &ProjectStructure{
		CCompiler:   ELF.CCompiler,
		Linker:      ELF.Linker,
		Folder:      uefiImageCreatorFolder,
		CodeFolder:  uefiImageCreatorFolder + "code",
		Environment: string(environment.Posix),
	},
	UEFI: &ProjectStructure{
		CCompiler:   EFI.CCompiler,
		Linker:      EFI.Linker,
		Folder:      uefiFolder,
		CodeFolder:  uefiFolder + "code",
		Environment: string(environment.Freestanding),
	},
	IMAGE_BUILDER: &ProjectStructure{
		CCompiler:   ELF.CCompiler,
		Linker:      ELF.Linker,
		Folder:      imageBuilderFolder,
		CodeFolder:  imageBuilderFolder + "code",
		Environment: string(environment.Posix),
	},
	SHARED: &ProjectStructure{
		CCompiler: ELF.CCompiler,
		// Is object file so adding standard for build output path
		Linker:      ELF.Linker,
		Folder:      sharedFolder,
		CodeFolder:  sharedFolder + "code",
		Environment: string(environment.Freestanding),
	},
	POSIX: &ProjectStructure{
		CCompiler: ELF.CCompiler,
		// Is object file so adding standard for build output path
		Linker:      ELF.Linker,
		Folder:      posixFolder,
		CodeFolder:  posixFolder + "code",
		Environment: string(environment.Posix),
	},
	PLATFORM_ABSTRACTION: &ProjectStructure{
		CCompiler: ELF.CCompiler,
		// Is object file so adding standard for build output path
		Linker:      ELF.Linker,
		Folder:      platformAbstractionFolder,
		CodeFolder:  platformAbstractionFolder + "code",
		Environment: string(environment.Freestanding),
	},
	X86: &ProjectStructure{
		CCompiler: ELF.CCompiler,
		// Is object file so adding standard for build output path
		Linker:      ELF.Linker,
		Folder:      x86Folder,
		CodeFolder:  x86Folder + "code",
		Environment: string(environment.Freestanding),
	},
}

func getConfiguredProjects() []string {
	var result = make([]string, 0)

	for name, _ := range PROJECT_STRUCTURES {
		result = append(result, name)
	}

	return result
}

var ConfiguredProjects = getConfiguredProjects()

func BuildOutputPath(cCompiler string, linker string, environment string, buildMode string) string {
	configurationPath := strings.Builder{}

	configurationPath.WriteString("build/")
	configurationPath.WriteString(fmt.Sprintf("%s/", cCompiler))
	configurationPath.WriteString(fmt.Sprintf("%s/", linker))
	configurationPath.WriteString(fmt.Sprintf("%s/", environment))
	configurationPath.WriteString(fmt.Sprintf("%s", buildMode))

	return configurationPath.String()
}

func BuildDirectoryRoot(project *ProjectStructure, buildMode string) string {
	buildDirectory := strings.Builder{}
	buildDirectory.WriteString(fmt.Sprintf("%s/", project.CodeFolder))
	buildDirectory.WriteString(BuildOutputPath(project.CCompiler, project.Linker, string(project.Environment), buildMode))

	return buildDirectory.String()
}

func BuildProjectTargetsFile(codeFolder string) string {
	projectTargetsFile := strings.Builder{}
	projectTargetsFile.WriteString(fmt.Sprintf("%s/build/targets.txt", codeFolder))
	return projectTargetsFile.String()
}

func GetAllProjects(selectedProjects []string) map[string]*ProjectStructure {
	if len(selectedProjects) == 0 {
		return PROJECT_STRUCTURES
	}

	result := make(map[string]*ProjectStructure)

	for _, name := range selectedProjects {
		var project = PROJECT_STRUCTURES[name]
		result[name] = project
	}

	return result
}