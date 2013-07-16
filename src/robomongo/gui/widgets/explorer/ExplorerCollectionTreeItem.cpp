#include "robomongo/gui/widgets/explorer/ExplorerCollectionTreeItem.h"

#include <QMessageBox>

#include "robomongo/core/AppRegistry.h"
#include "robomongo/gui/widgets/explorer/ExplorerDatabaseTreeItem.h"
#include "robomongo/core/domain/MongoCollection.h"
#include "robomongo/gui/GuiRegistry.h"
#include "robomongo/core/EventBus.h"
#include "robomongo/core/domain/MongoServer.h"
#include "robomongo/gui/dialogs/CreateDatabaseDialog.h"
#include "robomongo/gui/dialogs/DocumentTextEditor.h"
#include "robomongo/core/AppRegistry.h"
#include "robomongo/core/domain/App.h"

namespace
{
    const QString tooltipTemplate = QString(
        "%0 "
        "<table>"
        "<tr><td>Count:</td> <td><b>&nbsp;&nbsp;%1</b></td></tr>"
        "<tr><td>Size:</td><td><b>&nbsp;&nbsp;%2</b></td></tr>"
        "</table>"
        );
    void clearChildren(QTreeWidgetItem *root)
    {
        int itemCount = root->childCount();
        for (int i = 0; i < itemCount; ++i) {
            QTreeWidgetItem *item = root->child(0);
            root->removeChild(item);
            delete item;
        }
    }
}
namespace Robomongo
{
    R_REGISTER_EVENT(CollectionIndexesLoadingEvent)
    const QString ExplorerCollectionDirIndexesTreeItem::text = "Indexes";
    ExplorerCollectionDirIndexesTreeItem::ExplorerCollectionDirIndexesTreeItem(ExplorerCollectionTreeItem *const parent)
        :QTreeWidgetItem(parent)
    {
        setText(0, detail::buildName(text,0));
        setIcon(0, Robomongo::GuiRegistry::instance().folderIcon());
        setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    }

    ExplorerCollectionIndexesTreeItem::ExplorerCollectionIndexesTreeItem(const QString &val,ExplorerCollectionDirIndexesTreeItem *const parent)
        :QTreeWidgetItem(parent)
    {
        setText(0, val);
        setIcon(0, Robomongo::GuiRegistry::instance().indexIcon());
    }
    
    ExplorerCollectionTreeItem::ExplorerCollectionTreeItem(QTreeWidgetItem *parent,ExplorerDatabaseTreeItem *databaseItem,MongoCollection *collection) :
        QObject(),BaseClass(parent),_indexDir(new ExplorerCollectionDirIndexesTreeItem(this)),_collection(collection),_databaseItem(databaseItem)
    {
        QAction *addDocument = new QAction("Insert Document", this);
        connect(addDocument, SIGNAL(triggered()), SLOT(ui_addDocument()));

        QAction *updateDocument = new QAction("Update Documents", this);
        connect(updateDocument, SIGNAL(triggered()), SLOT(ui_updateDocument()));

        QAction *removeDocument = new QAction("Remove Documents", this);
        connect(removeDocument, SIGNAL(triggered()), SLOT(ui_removeDocument()));

        QAction *removeAllDocuments = new QAction("Remove All Documents", this);
        connect(removeAllDocuments, SIGNAL(triggered()), SLOT(ui_removeAllDocuments()));

        QAction *collectionStats = new QAction("Statistics", this);
        connect(collectionStats, SIGNAL(triggered()), SLOT(ui_collectionStatistics()));

        QAction *storageSize = new QAction("Storage Size", this);
        connect(storageSize, SIGNAL(triggered()), SLOT(ui_storageSize()));

        QAction *totalIndexSize = new QAction("Total Index Size", this);
        connect(totalIndexSize, SIGNAL(triggered()), SLOT(ui_totalIndexSize()));

        QAction *totalSize = new QAction("Total Size", this);
        connect(totalSize, SIGNAL(triggered()), SLOT(ui_totalSize()));

        QAction *shardVersion = new QAction("Shard Version", this);
        connect(shardVersion, SIGNAL(triggered()), SLOT(ui_shardVersion()));

        QAction *shardDistribution = new QAction("Shard Distribution", this);
        connect(shardDistribution, SIGNAL(triggered()), SLOT(ui_shardDistribution()));

        QAction *dropCollection = new QAction("Drop Collection", this);
        connect(dropCollection, SIGNAL(triggered()), SLOT(ui_dropCollection()));

        QAction *renameCollection = new QAction("Rename Collection", this);
        connect(renameCollection, SIGNAL(triggered()), SLOT(ui_renameCollection()));

        QAction *duplicateCollection = new QAction("Duplicate Collection", this);
        connect(duplicateCollection, SIGNAL(triggered()), SLOT(ui_duplicateCollection()));

        QAction *viewCollection = new QAction("View Documents", this);
        connect(viewCollection, SIGNAL(triggered()), SLOT(ui_viewCollection()));

        BaseClass::_contextMenu.addAction(viewCollection);
        BaseClass::_contextMenu.addSeparator();
        BaseClass::_contextMenu.addAction(addDocument);
        BaseClass::_contextMenu.addAction(updateDocument);
        BaseClass::_contextMenu.addAction(removeDocument);
        BaseClass::_contextMenu.addAction(removeAllDocuments);
        BaseClass::_contextMenu.addSeparator();
        BaseClass::_contextMenu.addAction(renameCollection);
        BaseClass::_contextMenu.addAction(duplicateCollection);
        BaseClass::_contextMenu.addAction(dropCollection);
        BaseClass::_contextMenu.addSeparator();
        BaseClass::_contextMenu.addAction(collectionStats);
        BaseClass::_contextMenu.addSeparator();
        BaseClass::_contextMenu.addAction(shardVersion);
        BaseClass::_contextMenu.addAction(shardDistribution);

        AppRegistry::instance().bus()->subscribe(_databaseItem, LoadCollectionIndexesResponse::Type, this);
        AppRegistry::instance().bus()->subscribe(_databaseItem, DeleteCollectionIndexResponse::Type, this);
        AppRegistry::instance().bus()->subscribe(this, CollectionIndexesLoadingEvent::Type, this);
        setText(0, _collection->name());
        setIcon(0, GuiRegistry::instance().collectionIcon());
        //setExpanded(true);
        addChild(_indexDir);
        setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        setToolTip(0, buildToolTip(collection));
    }

    void ExplorerCollectionTreeItem::handle(LoadCollectionIndexesResponse *event)
    {
        clearChildren(_indexDir);
        const QList<QString> &indexes = event->indexes();
        for(QList<QString>::const_iterator it=indexes.begin();it!=indexes.end();++it)
        {
            _indexDir->addChild(new ExplorerCollectionIndexesTreeItem(*it,_indexDir));
        }
        _indexDir->setText(0, detail::buildName(ExplorerCollectionDirIndexesTreeItem::text,_indexDir->childCount()));
    }

    void ExplorerCollectionTreeItem::handle(DeleteCollectionIndexResponse *event)
    {
        if(!event->index().isEmpty()){
            int itemCount = _indexDir->childCount();
            for (int i = 0; i < itemCount; ++i) {
                QTreeWidgetItem *item = _indexDir->child(i);
                if(item->text(0)==event->index())
                {
                    removeChild(item);
                    delete item;
                    break;
                }
            }
        }
        _indexDir->setText(0, detail::buildName(ExplorerCollectionDirIndexesTreeItem::text,_indexDir->childCount()));
    }
    void ExplorerCollectionTreeItem::handle(CollectionIndexesLoadingEvent *event)
    {
        _indexDir->setText(0, detail::buildName(ExplorerCollectionDirIndexesTreeItem::text,-1));
    }

    void ExplorerCollectionTreeItem::expand()
    {
         AppRegistry::instance().bus()->publish(new CollectionIndexesLoadingEvent(this));
         if(_databaseItem){
             _databaseItem->expandColection(this);
         }
    }

    void ExplorerCollectionTreeItem::deleteIndex(const QTreeWidgetItem * const ind)
    {
        if(_databaseItem){
            _databaseItem->deleteIndexFromCollection(this,ind->text(0));
        }
    }

    QString ExplorerCollectionTreeItem::buildToolTip(MongoCollection *collection)
    {	
        return tooltipTemplate.arg(collection->name()).arg(collection->info().count()).arg(collection->sizeString());
    }

    void ExplorerCollectionTreeItem::ui_addDocument()
    {
            MongoCollection *collection = this->collection();
            MongoDatabase *database = collection->database();
            MongoServer *server = database->server();
            ConnectionSettings *settings = server->connectionRecord();

            DocumentTextEditor editor(settings->getFullAddress(), database->name(),
                collection->name(), "{\n    \n}");

            editor.setCursorPosition(1, 4);
            editor.setWindowTitle("Insert Document");
            int result = editor.exec();
    #pragma message ("TODO: activateWindow")
            //activateWindow();

            if (result == QDialog::Accepted) {
                mongo::BSONObj obj = editor.bsonObj();
                server->insertDocument(obj, database->name(), collection->name());
            }
    }

    void ExplorerCollectionTreeItem::ui_removeDocument()
    {
        openCurrentCollectionShell(
            "remove({ '' : '' });"
            , false, CursorPosition(0, -10));
    }

    void ExplorerCollectionTreeItem::ui_removeAllDocuments()
    {
            MongoCollection *collection = this->collection();
            MongoDatabase *database = collection->database();
            // Ask user
    #pragma message ("TODO: Add parent to modal message")
            int answer = QMessageBox::question(NULL,
                "Remove All Documents",
                QString("Remove all documents from <b>%1</b> collection?").arg(collection->name()),
                QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

            if (answer == QMessageBox::Yes){
                MongoServer *server = database->server();
                mongo::BSONObjBuilder builder;
                mongo::BSONObj bsonQuery = builder.obj();
                mongo::Query query(bsonQuery);
                server->removeDocuments(query, database->name(), collection->name(), false);
            }
    }

    void ExplorerCollectionTreeItem::ui_updateDocument()
    {
        openCurrentCollectionShell(
            "update(\n"
            "    // query \n"
            "    {\n"
            "        \"key\" : \"value\"\n"
            "    },\n"
            "    \n"
            "    // update \n"
            "    {\n"
            "    },\n"
            "    \n"
            "    // options \n"
            "    {\n"
            "        \"multi\" : false,  // update only one document \n"
            "        \"upsert\" : false  // insert a new document, if no existing document match the query \n"
            "    }\n"
            ");", false);
    }

    void ExplorerCollectionTreeItem::ui_collectionStatistics()
    {
        openCurrentCollectionShell("stats()");
    }

    void ExplorerCollectionTreeItem::ui_dropCollection()
    {
            MongoCollection *collection = this->collection();
            MongoDatabase *database = collection->database();
            MongoServer *server = database->server();
            ConnectionSettings *settings = server->connectionRecord();
    #pragma message ("TODO: Add parent to modal message")
            // Ask user
            int answer = QMessageBox::question(NULL,
                "Drop Collection",
                QString("Drop <b>%1</b> collection?").arg(collection->name()),
                QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

            if (answer == QMessageBox::Yes){
                database->dropCollection(collection->name());
                database->loadCollections();
            }
    }

    void ExplorerCollectionTreeItem::ui_duplicateCollection()
    {
            MongoCollection *collection = this->collection();
            MongoDatabase *database = collection->database();
            MongoServer *server = database->server();
            ConnectionSettings *settings = server->connectionRecord();

            CreateDatabaseDialog dlg(settings->getFullAddress(),
                database->name(),
                collection->name());
            dlg.setWindowTitle("Duplicate Collection");
            dlg.setOkButtonText("&Duplicate");
            dlg.setInputLabelText("New Collection Name:");
            dlg.setInputText(collection->name() + "_copy");
            int result = dlg.exec();

            if (result == QDialog::Accepted) {
                database->duplicateCollection(collection->name(), dlg.databaseName());

                // refresh list of collections
                database->loadCollections();
            }
    }

    void ExplorerCollectionTreeItem::ui_renameCollection()
    {
            MongoCollection *collection = this->collection();
            MongoDatabase *database = collection->database();
            MongoServer *server = database->server();
            ConnectionSettings *settings = server->connectionRecord();

            CreateDatabaseDialog dlg(settings->getFullAddress(),
                database->name(),
                collection->name());
            dlg.setWindowTitle("Rename Collection");
            dlg.setOkButtonText("&Rename");
            dlg.setInputLabelText("New Collection Name:");
            dlg.setInputText(collection->name());
            int result = dlg.exec();

            if (result == QDialog::Accepted) {
                database->renameCollection(collection->name(), dlg.databaseName());
                // refresh list of collections
                database->loadCollections();
            }
    }

    void ExplorerCollectionTreeItem::ui_viewCollection()
    {
        openCurrentCollectionShell("find()");
    }

    void ExplorerCollectionTreeItem::ui_storageSize()
    {
        openCurrentCollectionShell("storageSize()");
    }

    void ExplorerCollectionTreeItem::ui_totalIndexSize()
    {
        openCurrentCollectionShell("totalIndexSize()");
    }

    void ExplorerCollectionTreeItem::ui_totalSize()
    {
        openCurrentCollectionShell("totalSize()");
    }

    void ExplorerCollectionTreeItem::ui_shardVersion()
    {
        openCurrentCollectionShell("getShardVersion()");
    }

    void ExplorerCollectionTreeItem::ui_shardDistribution()
    {
        openCurrentCollectionShell("getShardDistribution()");
    }

    void ExplorerCollectionTreeItem::openCurrentCollectionShell(const QString &script, bool execute,
        const CursorPosition &cursor)
    {
            MongoCollection *collection = this->collection();
            QString query = AppRegistry::instance().app()->buildCollectionQuery(collection->name(), script);
            AppRegistry::instance().app()->openShell(collection->database(), query, execute, collection->name(), cursor);
    }
}
