import bb.cascades 1.3
import bb.system 1.2
import YTVideoManagement 1.0

TabbedPane {
    id: tabbedPane
    
    Menu.definition: MenuDefinition {
        settingsAction: SettingsActionItem {
            onTriggered: {
                var format = AppSettings.preferredVideoFormat;
                var order  = AppSettings.videoSortOrder;
                
                if (format === 22) {
                    preferredVideoFormatDropDown.selectedIndex = 0;
                } else if (format === 18) {
                    preferredVideoFormatDropDown.selectedIndex = 1;
                } else {
                    preferredVideoFormatDropDown.selectedIndex = 0;
                }
                
                if (order === YTArrayDataModel.SortByTitle) {
                    sortOrderDropDown.selectedIndex = 0;
                } else if (order === YTArrayDataModel.SortBySize) {
                    sortOrderDropDown.selectedIndex = 1;
                } else {
                    sortOrderDropDown.selectedIndex = 0;
                }
                
                showUnwatchedFirstToggleButton.checked = AppSettings.showUnwatchedFirst;
                autoRepeatPlaybackToggleButton.checked = AppSettings.autoRepeatPlayback;
                
                settingsSheet.open();
            }
            
            attachedObjects: [
                Sheet {
                    id: settingsSheet
                    
                    Page {
                        id: settingsPage
                        
                        titleBar: TitleBar {
                            title: qsTr("Settings")
                            
                            acceptAction: ActionItem {
                                title: qsTr("OK")
                                
                                onTriggered: {
                                    var format = AppSettings.preferredVideoFormat;
                                    var order  = AppSettings.videoSortOrder;
                                    
                                    if (preferredVideoFormatDropDown.selectedIndex === 0) {
                                        format = 22;
                                    } else if (preferredVideoFormatDropDown.selectedIndex === 1) {
                                        format = 18;
                                    }
                                    
                                    if (sortOrderDropDown.selectedIndex === 0) {
                                        order = YTArrayDataModel.SortByTitle;
                                    } else if (sortOrderDropDown.selectedIndex === 1) {
                                        order = YTArrayDataModel.SortBySize;
                                    }
                                    
                                    AppSettings.preferredVideoFormat = format;
                                    AppSettings.videoSortOrder       = order;
                                    AppSettings.showUnwatchedFirst   = showUnwatchedFirstToggleButton.checked;
                                    AppSettings.autoRepeatPlayback   = autoRepeatPlaybackToggleButton.checked;
                                    
                                    YTVideoManager.setPreferredVideoFormat(format);
                                    cacheListViewDataModel.setUnwatchedFirst(AppSettings.showUnwatchedFirst);
                                    cacheListViewDataModel.setSortOrder(order);
                                    
                                    settingsSheet.close();
                                }
                            }
                            
                            dismissAction: ActionItem {
                                title: qsTr("Cancel")
                                
                                onTriggered: {
                                    settingsSheet.close();
                                }
                            }
                        }
                        
                        Container {
                            background: Color.White
                            
                            ScrollView {
                                accessibility.name: qsTr("Settings")
                                
                                scrollViewProperties {
                                    scrollMode: ScrollMode.Vertical
                                }
                                
                                Container {
                                    background:   Color.Transparent
                                    leftPadding:  ui.sdu(1)
                                    rightPadding: ui.sdu(1)
                                    
                                    layout: StackLayout {
                                    }
                                    
                                    Divider {
                                        accessibility.name: qsTr("Divider")
                                    }
                                    
                                    DropDown {
                                        id:    preferredVideoFormatDropDown
                                        title: qsTr("Preferred Video Format")
                                        
                                        Option {
                                            text: qsTr("720p H.264 MP4")
                                        }
                                        
                                        Option {
                                            text: qsTr("360p H.264 MP4")
                                        }
                                    }
                                    
                                    DropDown {
                                        id:    sortOrderDropDown
                                        title: qsTr("Sort Order")
                                        
                                        Option {
                                            text: qsTr("By Title")
                                        }
                                        
                                        Option {
                                            text: qsTr("By Size")
                                        }
                                    }
                                    
                                    Divider {
                                        accessibility.name: qsTr("Divider")
                                    }
                                    
                                    Container {
                                        background: Color.Transparent
                                        
                                        layout: StackLayout {
                                            orientation: LayoutOrientation.LeftToRight
                                        }
                                        
                                        Label {
                                            id:                 showUnwatchedFirstLabel
                                            verticalAlignment:  VerticalAlignment.Center
                                            multiline:          true
                                            textStyle.color:    Color.Black
                                            textStyle.fontSize: FontSize.Medium
                                            text:               qsTr("Show Unwatched Video First")
                                            
                                            layoutProperties: StackLayoutProperties {
                                                spaceQuota: 1
                                            }
                                        }
                                        
                                        ToggleButton {
                                            id:                       showUnwatchedFirstToggleButton
                                            accessibility.labelledBy: showUnwatchedFirstLabel
                                            
                                            layoutProperties: StackLayoutProperties {
                                                spaceQuota: -1
                                            }
                                        }
                                    }
                                    
                                    Container {
                                        background: Color.Transparent
                                        
                                        layout: StackLayout {
                                            orientation: LayoutOrientation.LeftToRight
                                        }
                                        
                                        Label {
                                            id:                 autoRepeatPlaybackLabel
                                            verticalAlignment:  VerticalAlignment.Center
                                            multiline:          true
                                            textStyle.color:    Color.Black
                                            textStyle.fontSize: FontSize.Medium
                                            text:               qsTr("Auto Repeat Playback")
                                            
                                            layoutProperties: StackLayoutProperties {
                                                spaceQuota: 1
                                            }
                                        }
                                        
                                        ToggleButton {
                                            id:                       autoRepeatPlaybackToggleButton
                                            accessibility.labelledBy: autoRepeatPlaybackLabel
                                            
                                            layoutProperties: StackLayoutProperties {
                                                spaceQuota: -1
                                            }
                                        }
                                    }
                                    
                                    Divider {
                                        accessibility.name: qsTr("Divider")
                                    }
                                }
                            }
                        }
                    }
                }
            ]
        }
        
        helpAction: HelpActionItem {
            onTriggered: {
                helpWebView.url = qsTr("local:///assets/doc/help.html");
                
                helpSheet.open();
            }
            
            attachedObjects: [
                Sheet {
                    id: helpSheet
                    
                    Page {
                        titleBar: TitleBar {
                            title: qsTr("Help")
                            
                            dismissAction: ActionItem {
                                title: qsTr("Close")
                                
                                onTriggered: {
                                    helpSheet.close();
                                }
                            }
                        }
                        
                        actions: [
                            ActionItem {
                                title:               qsTr("Review App")
                                imageSource:         "images/review.png"
                                ActionBar.placement: ActionBarPlacement.Signature
                                
                                onTriggered: {
                                    appWorldInvocation.trigger("bb.action.OPEN");
                                }
                                
                                attachedObjects: [
                                    Invocation {
                                        id: appWorldInvocation
                                        
                                        query: InvokeQuery {
                                            mimeType: "application/x-bb-appworld"
                                            uri:      "appworld://content/42093887"
                                        }
                                    }
                                ]
                            }
                        ]
                        
                        ScrollView {
                            accessibility.name: qsTr("Help browser")
                            
                            scrollViewProperties {
                                scrollMode:         ScrollMode.Both
                                pinchToZoomEnabled: true
                                minContentScale:    1.0
                                maxContentScale:    4.0
                            }
                            
                            WebView {
                                id:                 helpWebView
                                url:                qsTr("local:///assets/doc/help.html")
                                accessibility.name: qsTr("Help browser")
                            }
                        }
                    }
                }
            ]
        }
    }
    
    Tab {
        id:          youTubeTab
        title:       qsTr("YouTube")
        imageSource: "images/youtube.png"
        
        Page {
            function onInvocationVideoUrlChanged() {
                var video_id = YTVideoManager.getVideoId(InvocationHelper.videoUrl);
                
                if (video_id !== "") {
                    youTubeWebView.url = "http://m.youtube.com/watch?v=" + encodeURIComponent(video_id);
                }
            }
            
            onCreationCompleted: {
                var video_id = YTVideoManager.getVideoId(InvocationHelper.videoUrl);
                
                if (video_id !== "") {
                    youTubeWebView.url = "http://m.youtube.com/watch?v=" + encodeURIComponent(video_id);
                }
                
                InvocationHelper.videoUrlChanged.connect(onInvocationVideoUrlChanged);
            }
            
            actions: [
                ActionItem {
                    title:               qsTr("Back")
                    imageSource:         "images/back.png"
                    ActionBar.placement: ActionBarPlacement.OnBar
                    
                    onTriggered: {
                        youTubeWebView.goBack();
                    }
                },
                ActionItem {
                    title:               qsTr("Cache")
                    imageSource:         "images/cache.png"
                    ActionBar.placement: ActionBarPlacement.Signature
                    
                    onTriggered: {
                        var video_id = YTVideoManager.getVideoId(youTubeWebView.url);
                        
                        if (video_id !== "") {
                            if (YTVideoManager.addTask(video_id)) {
                                cachingStartedToast.show();
                            } else {
                                cachingFailedToast.show();
                            }
                        }
                    }
                    
                    attachedObjects: [
                        SystemToast {
                            id:   cachingStartedToast
                            body: qsTr("Video caching started") 
                        },
                        SystemToast {
                            id:   cachingFailedToast
                            body: qsTr("Cannot cache this video. May be it is already cached?")
                        }
                    ]
                },
                ActionItem {
                    title:               qsTr("Home")
                    imageSource:         "images/home.png"
                    ActionBar.placement: ActionBarPlacement.OnBar
                    
                    onTriggered: {
                        youTubeWebView.url = "http://m.youtube.com/";
                    }
                },
                ActionItem {
                    title:               qsTr("Reload")
                    imageSource:         "images/reload.png"
                    ActionBar.placement: ActionBarPlacement.InOverflow
                    
                    onTriggered: {
                        youTubeWebView.reload();
                    }
                }
            ]
            
            Container {
                background: Color.White
                
                layout: DockLayout {
                }
                
                ScrollView {
                    horizontalAlignment: HorizontalAlignment.Fill
                    verticalAlignment:   VerticalAlignment.Fill
                    accessibility.name:  qsTr("YouTube browser")
                    
                    scrollViewProperties {
                        scrollMode:         ScrollMode.Both
                        pinchToZoomEnabled: true
                        minContentScale:    1.0
                        maxContentScale:    4.0
                    }
                    
                    WebView {
                        id:                 youTubeWebView
                        url:                "http://m.youtube.com/"
                        accessibility.name: qsTr("YouTube browser")
                    }
                }
                
                ProgressIndicator {
                    horizontalAlignment: HorizontalAlignment.Fill
                    verticalAlignment:   VerticalAlignment.Bottom
                    visible:             youTubeWebView.loading
                    fromValue:           0
                    toValue:             100
                    value:               youTubeWebView.loadProgress
                    accessibility.name:  qsTr("Page load progress: %1%").arg(value.toFixed())
                }
            }
        }
    }
    
    Tab {
        title:       qsTr("Cached Video")
        imageSource: "images/cache.png"
        
        Page {
            titleBar: TitleBar {
                title: qsTr("Cached Video")
            }
            
            Container {
                background: Color.White
                
                layout: DockLayout {
                }
                
                Container {
                    horizontalAlignment: HorizontalAlignment.Center
                    verticalAlignment:   VerticalAlignment.Center
                    background:          Color.Transparent
                    visible:             cacheListViewDataModel.taskCount <= 0
                    
                    layout: StackLayout {
                    }
                    
                    ImageView {
                        horizontalAlignment:      HorizontalAlignment.Center
                        imageSource:              "images/cache_empty.png"
                        accessibility.labelledBy: cacheEmptyLabel
                    }
                    
                    Label {
                        id:                  cacheEmptyLabel
                        horizontalAlignment: HorizontalAlignment.Center
                        textStyle.color:     Color.Black
                        textStyle.fontSize:  FontSize.XLarge
                        text:                qsTr("No cached video")
                    }
                }
                
                Container {
                    horizontalAlignment: HorizontalAlignment.Fill
                    verticalAlignment:   VerticalAlignment.Fill
                    background:          Color.Transparent
                    visible:             cacheListViewDataModel.taskCount > 0
                    
                    layout: StackLayout {
                    }
                    
                    ListView {
                        horizontalAlignment: HorizontalAlignment.Fill
                        accessibility.name:  qsTr("List of cached video")
                        
                        property variant appSettings:    AppSettings 
                        property variant ytVideoManager: YTVideoManager
                        
                        dataModel: YTArrayDataModel {
                            id:           cacheListViewDataModel
                            videoManager: YTVideoManager
                        }
                        
                        layoutProperties: StackLayoutProperties {
                            spaceQuota: 1
                        }
                        
                        function navigateToWebPage(url) {
                            youTubeWebView.url = url;
                            
                            tabbedPane.activeTab = youTubeTab;
                        } 
                        
                        onCreationCompleted: {
                            cacheListViewDataModel.setUnwatchedFirst(AppSettings.showUnwatchedFirst);
                            cacheListViewDataModel.setSortOrder(AppSettings.videoSortOrder);
                        }
                        
                        listItemComponents: [
                            ListItemComponent {
                                type: ""
                                
                                Container {
                                    id:           itemRoot
                                    background:   itemSelected ? Color.create("#00A7DE") : Color.White 
                                    leftPadding:  ui.sdu(1)
                                    rightPadding: ui.sdu(1)
                                    
                                    property bool   itemSelected: ListItem.active || ListItem.selected
                                    property int    itemState:    ListItemData.state
                                    property int    itemSize:     ListItemData.size
                                    property int    itemDone:     ListItemData.done
                                    property string itemVideoId:  ListItemData.videoId
                                    property string itemTitle:    ListItemData.title
                                    property string itemErrorMsg: ListItemData.errorMsg
                                    
                                    layout: StackLayout {
                                    }
                                    
                                    gestureHandlers: [
                                        TapHandler {
                                            onTapped: {
                                                if (itemRoot.itemState === YTDownloadState.StateCompleted) {
                                                    playerNavigationPane.push(playerPageDefinition.createObject());
                                                    
                                                    playerSheet.open();
                                                }
                                            }
                                            
                                            attachedObjects: [
                                                Sheet {
                                                    id:          playerSheet
                                                    peekEnabled: false
                                                    
                                                    onOpened: {
                                                        var page = playerNavigationPane.top;
                                                        
                                                        if (page.objectName === "playerPage") {
                                                            page.playVideo(itemRoot.ListItem.view.ytVideoManager.getTaskVideoURI(itemRoot.itemVideoId), itemRoot.itemTitle, itemRoot.ListItem.view.appSettings.autoRepeatPlayback);
                                                        }
                                                    }
                                                    
                                                    NavigationPane {
                                                        id:          playerNavigationPane
                                                        peekEnabled: false
                                                        
                                                        onTopChanged: {
                                                            if (playerSheet.opened) {
                                                                if (page.objectName === "playerPage") {
                                                                    page.playVideo(itemRoot.ListItem.view.ytVideoManager.getTaskVideoURI(itemRoot.itemVideoId), itemRoot.itemTitle, itemRoot.ListItem.view.appSettings.autoRepeatPlayback);
                                                                } else {
                                                                    playerSheet.close();
                                                                }
                                                            }
                                                        }
                                                        
                                                        onPopTransitionEnded: {
                                                            if (page.objectName === "playerPage") {
                                                                page.disconnectSignals();
                                                            }
                                                            
                                                            page.destroy();
                                                        }
                                                        
                                                        Page {
                                                            Container {
                                                                background: Color.White
                                                            }
                                                        }
                                                    }
                                                    
                                                    attachedObjects: [
                                                        ComponentDefinition {
                                                            id:     playerPageDefinition
                                                            source: "PlayerPage.qml"
                                                        }
                                                    ]
                                                }
                                            ]
                                        }
                                    ]
                                    
                                    contextActions: [
                                        ActionSet {
                                            ActionItem {
                                                title:       qsTr("Pause/Resume")
                                                imageSource: "images/pause.png"
                                                enabled:     itemRoot.itemState !== YTDownloadState.StateCompleted
                                                
                                                onTriggered: {
                                                    if (itemRoot.itemState === YTDownloadState.StatePaused) {
                                                        itemRoot.ListItem.view.ytVideoManager.resumeTask(itemRoot.itemVideoId);
                                                    } else {
                                                        itemRoot.ListItem.view.ytVideoManager.pauseTask(itemRoot.itemVideoId);
                                                    }
                                                }
                                            }
                                            
                                            ActionItem {
                                                title:       qsTr("Open YouTube Page")
                                                imageSource: "images/youtube.png"
                                                
                                                onTriggered: {
                                                    itemRoot.ListItem.view.navigateToWebPage(itemRoot.ListItem.view.ytVideoManager.getTaskWebURL(itemRoot.itemVideoId));
                                                }
                                            }
                                            
                                            DeleteActionItem {
                                                title:       qsTr("Delete")
                                                imageSource: "images/delete.png"
                                                
                                                onTriggered: {
                                                    itemRoot.ListItem.view.ytVideoManager.delTask(itemRoot.itemVideoId);
                                                    
                                                    videoDeletedToast.deletedVideoId = itemRoot.itemVideoId;
                                                    videoDeletedToast.show();
                                                }
                                                
                                                attachedObjects: [
                                                    SystemToast {
                                                        id:             videoDeletedToast
                                                        body:           qsTr("Video deleted successfully")
                                                        button.label:   qsTr("Undo")
                                                        button.enabled: true
                                                        
                                                        property string deletedVideoId: ""
                                                        
                                                        onFinished: {
                                                            if (buttonSelection() === button) {
                                                                if (itemRoot.ListItem.view.ytVideoManager.restTask(deletedVideoId)) {
                                                                    videoRestoredToast.show();
                                                                } else {
                                                                    videoRestoreFailedToast.show();
                                                                }
                                                            }
                                                        }
                                                    },
                                                    SystemToast {
                                                        id:   videoRestoredToast
                                                        body: qsTr("Video restored successfully")
                                                    },
                                                    SystemToast {
                                                        id: videoRestoreFailedToast
                                                        body: qsTr("Cannot restore video")
                                                    }
                                                ]
                                            }
                                        }
                                    ]
                                    
                                    Label {
                                        multiline:            true
                                        textFormat:           TextFormat.Plain
                                        textStyle.color:      itemRoot.itemSelected ? Color.White : Color.Black
                                        textStyle.fontWeight: FontWeight.Bold
                                        textStyle.fontSize:   FontSize.Medium
                                        text:                 itemRoot.itemTitle
                                    }
                                    
                                    ProgressIndicator {
                                        visible:   itemRoot.itemState === YTDownloadState.StateActive
                                        fromValue: 0
                                        toValue:   itemRoot.itemSize
                                        value:     itemRoot.itemDone
                                        accessibility.name:  qsTr("Video load progress: %1%").arg(value.toFixed())
                                    }
                                    
                                    Label {
                                        visible:             itemRoot.itemState !== YTDownloadState.StateActive
                                        multiline:           true
                                        textStyle.color:     itemRoot.itemState === YTDownloadState.StateError ? Color.Red : (itemRoot.itemSelected ? Color.LightGray : Color.DarkGray)
                                        textStyle.fontStyle: FontStyle.Italic
                                        textStyle.fontSize:  FontSize.Small
                                        text:                itemRoot.itemState === YTDownloadState.StateCompleted ? (itemRoot.itemSize / 1048576).toFixed(2) + " MiB" :
                                        (itemRoot.itemState === YTDownloadState.StateError     ? itemRoot.itemErrorMsg                             :
                                        (itemRoot.itemState === YTDownloadState.StateQueued    ? qsTr("QUEUED")                                    :
                                        (itemRoot.itemState === YTDownloadState.StatePaused    ? qsTr("PAUSED")                                    : "")))
                                    }
                                    
                                    Divider {
                                        accessibility.name: qsTr("Divider")
                                    }
                                }
                            }
                        ]
                    }
                    
                    TextField {
                        id:                  filterTextField
                        horizontalAlignment: HorizontalAlignment.Fill
                        hintText:            qsTr("filter string")
                        
                        layoutProperties: StackLayoutProperties {
                            spaceQuota: -1
                        }
                        
                        input {
                            submitKey: SubmitKey.Search
                            
                            onSubmitted: {
                                cacheListViewDataModel.setFilter(text);
                            }
                        }
                        
                        onTextChanging: {
                            cacheListViewDataModel.setFilter(text);
                        }
                    }
                }
            }
        }
    }
}
