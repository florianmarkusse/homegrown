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

var EFI_SYSTEM = CommonConfig{
	CCompiler: "clang-19",
	Linker:    "lld-link-19",
}

// If you add a project add it here
const KERNEL = "kernel"
const EFI_TO_KERNEL = "efi-to-kernel"
const OS_LOADER = "os-loader"
const EFI = "efi"
const IMAGE_BUILDER = "image-builder"
const SHARED = "shared"
const POSIX = "posix"
const X86 = "x86"
const X86_PHYSICAL = "x86-physical"
const X86_VIRTUAL = "x86-virtual"
const X86_EFI = "x86-efi"
const X86_POLICY = "x86-policy"
const UEFI = "uefi"
const FREESTANDING = "freestanding"
const ABSTRACTION = "abstraction"

// and here
var kernelFolder = common.REPO_PROJECTS + "/" + KERNEL + "/"
var efiToKernelFolder = common.REPO_PROJECTS + "/" + EFI_TO_KERNEL + "/"
var osLoaderFolder = common.REPO_PROJECTS + "/" + OS_LOADER + "/"
var efiFolder = common.REPO_PROJECTS + "/" + EFI + "/"
var imageBuilderFolder = common.REPO_PROJECTS + "/" + IMAGE_BUILDER + "/"
var sharedFolder = common.REPO_PROJECTS + "/" + SHARED + "/"
var posixFolder = common.REPO_PROJECTS + "/" + POSIX + "/"
var x86Folder = common.REPO_PROJECTS + "/" + X86 + "/"
var x86PhysicalFolder = common.REPO_PROJECTS + "/" + X86_PHYSICAL + "/"
var x86VirtualFolder = common.REPO_PROJECTS + "/" + X86_VIRTUAL + "/"
var x86EfiFolder = common.REPO_PROJECTS + "/" + X86_EFI + "/"
var x86PolicyFolder = common.REPO_PROJECTS + "/" + X86_POLICY + "/"
var uefiFolder = common.REPO_PROJECTS + "/" + UEFI + "/"
var freestandingFolder = common.REPO_PROJECTS + "/" + FREESTANDING + "/"
var abstractionFolder = common.REPO_PROJECTS + "/" + ABSTRACTION + "/"

// and here
var PROJECT_STRUCTURES = map[string]*ProjectStructure{
	KERNEL: {
		CCompiler:   ELF.CCompiler,
		Linker:      ELF.Linker,
		Folder:      kernelFolder,
		CodeFolder:  kernelFolder + "code",
		Environment: string(environment.Freestanding),
	},
	EFI_TO_KERNEL: {
		CCompiler:   ELF.CCompiler,
		Linker:      ELF.Linker,
		Folder:      efiToKernelFolder,
		CodeFolder:  efiToKernelFolder + "code",
		Environment: string(environment.Efi),
	},
	OS_LOADER: {
		CCompiler:   EFI_SYSTEM.CCompiler,
		Linker:      EFI_SYSTEM.Linker,
		Folder:      osLoaderFolder,
		CodeFolder:  osLoaderFolder + "code",
		Environment: string(environment.Efi),
	},
	EFI: {
		CCompiler:   EFI_SYSTEM.CCompiler,
		Linker:      EFI_SYSTEM.Linker,
		Folder:      efiFolder,
		CodeFolder:  efiFolder + "code",
		Environment: string(environment.Efi),
	},
	IMAGE_BUILDER: {
		CCompiler:   ELF.CCompiler,
		Linker:      ELF.Linker,
		Folder:      imageBuilderFolder,
		CodeFolder:  imageBuilderFolder + "code",
		Environment: string(environment.Posix),
	},
	SHARED: {
		CCompiler:   ELF.CCompiler,
		Linker:      ELF.Linker,
		Folder:      sharedFolder,
		CodeFolder:  sharedFolder + "code",
		Environment: string(environment.Freestanding),
	},
	POSIX: {
		CCompiler:   ELF.CCompiler,
		Linker:      ELF.Linker,
		Folder:      posixFolder,
		CodeFolder:  posixFolder + "code",
		Environment: string(environment.Posix),
	},
	X86: {
		CCompiler:   ELF.CCompiler,
		Linker:      ELF.Linker,
		Folder:      x86Folder,
		CodeFolder:  x86Folder + "code",
		Environment: string(environment.Freestanding),
	},
	X86_PHYSICAL: {
		CCompiler:   ELF.CCompiler,
		Linker:      ELF.Linker,
		Folder:      x86PhysicalFolder,
		CodeFolder:  x86PhysicalFolder + "code",
		Environment: string(environment.Freestanding),
	},
	X86_VIRTUAL: {
		CCompiler:   ELF.CCompiler,
		Linker:      ELF.Linker,
		Folder:      x86VirtualFolder,
		CodeFolder:  x86VirtualFolder + "code",
		Environment: string(environment.Freestanding),
	},
	X86_POLICY: {
		CCompiler:   ELF.CCompiler,
		Linker:      ELF.Linker,
		Folder:      x86PolicyFolder,
		CodeFolder:  x86PolicyFolder + "code",
		Environment: string(environment.Freestanding),
	},
	X86_EFI: {
		CCompiler:   EFI_SYSTEM.CCompiler,
		Linker:      EFI_SYSTEM.Linker,
		Folder:      x86EfiFolder,
		CodeFolder:  x86EfiFolder + "code",
		Environment: string(environment.Efi),
	},
	UEFI: {
		CCompiler:   ELF.CCompiler,
		Linker:      ELF.Linker,
		Folder:      uefiFolder,
		CodeFolder:  uefiFolder + "code",
		Environment: string(environment.Freestanding),
	},
	FREESTANDING: {
		CCompiler:   ELF.CCompiler,
		Linker:      ELF.Linker,
		Folder:      freestandingFolder,
		CodeFolder:  freestandingFolder + "code",
		Environment: string(environment.Freestanding),
	},
	ABSTRACTION: {
		CCompiler:   ELF.CCompiler,
		Linker:      ELF.Linker,
		Folder:      abstractionFolder,
		CodeFolder:  abstractionFolder,
		Environment: string(environment.Freestanding),
	},
}

func getConfiguredProjects() []string {
	var result = make([]string, 0)

	for name := range PROJECT_STRUCTURES {
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
	configurationPath.WriteString(buildMode)

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
