#include "pch.h"
#include "App/Layer/FileLayer.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Core/AssetManager.h"
#include "Events/EventBus.h"

#include "NIRS/Anatomy/AnatomyManager.h"

#include "NIRS/Snirf.h"
#include "GUI/SNIRFFileLoaderPanel.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include <optional>

FileLayer::FileLayer() 
{
}

FileLayer::~FileLayer()
{
}
void FileLayer::OnAttach()
{
    // Initialize file loader panel (assuming this is necessary for your app)
    m_SNIRFFileLoaderPanel = CreateRef<SNIRFFileLoaderPanel>();

    try
    {
        // Use a more descriptive name for your application's database
        SQLite::Database db("fNIRS_Catalog.db3", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

        // --- Sophisticated Schema Setup ---

        // 1. Subjects Table: Stores unique subject metadata
        db.exec("DROP TABLE IF EXISTS Subjects;");
        db.exec("CREATE TABLE Subjects ("
            "   SubjectID TEXT PRIMARY KEY NOT NULL,"
            "   Age INTEGER,"
            "   Gender TEXT,"
            "   Handedness TEXT"
            ");");

        // 2. Files Table: Indexes the SNIRF files and their key metadata
        db.exec("DROP TABLE IF EXISTS Files;");
        db.exec("CREATE TABLE Files ("
            "   FileID INTEGER PRIMARY KEY AUTOINCREMENT,"
            "   SubjectID TEXT NOT NULL,"
            "   FilePath TEXT NOT NULL UNIQUE,"
            "   SessionName TEXT,"
            "   AnalysisType TEXT," // e.g., 'Raw', 'OD', 'HRF_GLM'
            "   RecordingDate TEXT,"
            "   FOREIGN KEY(SubjectID) REFERENCES Subjects(SubjectID)"
            ");");

        // 3. Sequences Table: Manages the ordering and dependencies for analysis pipelines
        db.exec("DROP TABLE IF EXISTS AnalysisSequences;");
        db.exec("CREATE TABLE AnalysisSequences ("
            "   SequenceID INTEGER PRIMARY KEY AUTOINCREMENT,"
            "   AnalysisName TEXT NOT NULL,"
            "   StepOrder INTEGER NOT NULL," // The required order of files/steps
            "   InputFileID INTEGER NOT NULL," // References the FileID to be used at this step
            "   FOREIGN KEY(InputFileID) REFERENCES Files(FileID)"
            ");");

        // --- Data Population (Simulated) ---

        SQLite::Transaction transaction(db);
        // Prepare statement for Subject insertion
        SQLite::Statement insertSubject(db, "INSERT OR IGNORE INTO Subjects (SubjectID, Age, Gender) VALUES (?, ?, ?)");

        // --- Subject 1 Insertion ---
        insertSubject.bind(1, "sub-001");
        insertSubject.bind(2, 25);
        insertSubject.bind(3, "M");
        insertSubject.exec();
        insertSubject.reset(); // Clear bindings for next use

        // --- Subject 2 Insertion (Example of reuse) ---
        insertSubject.bind(1, "sub-002");
        insertSubject.bind(2, 30);
        insertSubject.bind(3, "F");
        insertSubject.exec();
        insertSubject.reset();

        // Similarly for the Files statement:
        SQLite::Statement insertFile(db, "INSERT INTO Files (SubjectID, FilePath, SessionName, AnalysisType) VALUES (?, ?, ?, ?)");
        insertFile.bind(1, "sub-001");
		insertFile.bind(2, "/data/sub-001/ses-01/raw.snirf");
		insertFile.bind(3, "ses-01");
		insertFile.bind(4, "Raw");
		insertFile.exec();
		//insertFile.reset();

        //
        //// --- File 1 Insertion ---
        //insertFile.bind(1, "sub-001").bind(2, "/data/sub-001/ses-01/raw.snirf").bind(3, "ses-01").bind(4, "Raw");
        //insertFile.exec();

        std::cout << "Database schema created and initial data inserted successfully." << std::endl;
    }
    catch (std::exception& e)
    {
        std::cout << "Setup exception: " << e.what() << std::endl;
    }

    // --- Data Querying (Advanced) ---

    try
    {
        SQLite::Database db("fNIRS_Catalog.db3");

        // Query to find the file paths for all 'Raw' data for 'sub-001'
        SQLite::Statement query(db, "SELECT F.FilePath, S.Age FROM Files F JOIN Subjects S ON F.SubjectID = S.SubjectID WHERE F.SubjectID = ? AND F.AnalysisType = ?");

        query.bind(1, "sub-001");
        query.bind(2, "Raw");

        std::cout << "\n--- Sophisticated Query Results (Raw files for sub-001) ---" << std::endl;
        while (query.executeStep())
        {
            // Column 0 is FilePath, Column 1 is Age
            std::string filePath = query.getColumn(0).getText();
            int age = query.getColumn(1).getInt();

            std::cout << "Path: " << filePath << ", Subject Age: " << age << std::endl;
        }
        std::cout << "-------------------------------------------------------------" << std::endl;
    }
    catch (std::exception& e)
    {
        std::cout << "Query exception: " << e.what() << std::endl;
    }

    PostInit();
}
void FileLayer::OnDetach()
{
}

void FileLayer::OnUpdate(float dt)
{
}

void FileLayer::OnRender()
{
}

void FileLayer::OnImGuiRender()
{
	if (m_SNIRFileLoaderOpen) m_SNIRFFileLoaderPanel->OnImGuiRender(true, m_SNIRFileLoaderOpen);
}

void FileLayer::OnEvent(Event& event)
{
}

void FileLayer::RenderMenuBar()
{
	if (ImGui::BeginMenu("File")) {

		if (ImGui::MenuItem("Open fNIRS file")) {
			m_SNIRFileLoaderOpen = !m_SNIRFileLoaderOpen;
		}

		if (ImGui::MenuItem("Open Head Anatomy")) LoadHeadAnatomy();
		if (ImGui::MenuItem("Open Cortex Anatomy")) LoadCortexAnatomy();

		if (ImGui::MenuItem("Exit")) EventBus::Instance().Publish<ExitApplicationCommand>({});

		ImGui::EndMenu();
	}
}

void FileLayer::PostInit()
{

    std::string headFilepath = "C:/dev/NIRSViz/Assets/Models/head_model_2.obj";
    std::string cortexFilepath = "C:/dev/NIRSViz/Assets/Models/cortex_model.obj";

    NIRS::AnatomyManager::Instance().LoadCortex(cortexFilepath);
    NIRS::AnatomyManager::Instance().LoadHead(headFilepath);
	std::string snirfFilepath = "C:/dev/NIRSViz/Assets/NIRS/sub01_trial03_TRIM_BP_ZNORM_TDDR.snirf";


	AssetManager::Register<SNIRF>("SNIRF", CreateRef<SNIRF>(std::string(snirfFilepath)));
	EventBus::Instance().Publish<OnSNIRFLoaded>({});
}

void FileLayer::LoadSNIRFFile()
{
	char filePath[MAX_PATH] = "";
	OPENFILENAMEA ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = filePath;
	ofn.nMaxFile = sizeof(filePath);
	ofn.lpstrFilter = "SNIRF Files (*.snirf)\0*.snirf\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (!GetOpenFileNameA(&ofn)) return;
	
    auto path = std::filesystem::path(filePath);
	SNIRF snirf(path);

	AssetManager::Register<SNIRF>("SNIRF", CreateRef<SNIRF>(snirf));

	EventBus::Instance().Publish<OnSNIRFLoaded>({});
}

namespace Utils {
        std::optional<std::string> GetFile(const std::string& filter) {
            char filePath[MAX_PATH] = "";
            OPENFILENAMEA ofn;
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = NULL;
            ofn.lpstrFile = filePath;
            ofn.nMaxFile = sizeof(filePath);
            ofn.lpstrFilter = filter.c_str();
            ofn.nFilterIndex = 1;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            if (!GetOpenFileNameA(&ofn)) return std::nullopt;
            else  return std::string(filePath);
		}
}

void FileLayer::LoadHeadAnatomy()
{
	auto path = Utils::GetFile("OBJ Files (*.obj)\0*.obj\0All Files (*.*)\0*.*\0");
	
	if (path.has_value()) 
    {
        NIRS::AnatomyManager::Instance().LoadHead(path.value());
    }
}

void FileLayer::LoadCortexAnatomy()
{
    auto path = Utils::GetFile("OBJ Files (*.obj)\0*.obj\0All Files (*.*)\0*.*\0");

    if (path.has_value())
    {
        NIRS::AnatomyManager::Instance().LoadCortex(path.value());
    }
}

