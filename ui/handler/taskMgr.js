
var totalTaskTNAM = 0;
var totalTaskRNAM = 0;


function addTaskTNAM(){
    totalTaskTNAM++;
    var tnamCompont = Qt.createComponent("../ComponentTNAM.qml",Component.PreferSynchronous);
    //console.log(tnamCompont.errorString());
    var objTnam = tnamCompont.createObject(sl,{
                                               idx: totalTaskTNAM
                                           });

    var Tabcomponent = Qt.createComponent("../CustomTabButton.qml");

    var objTab = Tabcomponent.createObject(bar,{
                                               text: "TNAM Task #" + totalTaskTNAM,
                                               tnamObj: objTnam
                                           });
    bar.currentIndex = bar.count - 1;
}

function addTaskRNAM(){
    totalTaskRNAM++;
    var rnamCompont = Qt.createComponent("../ComponentRNAM.qml",Component.PreferSynchronous);
    var objRnam = rnamCompont.createObject(sl,{
                                               idx: totalTaskRNAM
                                           });

    var Tabcomponent = Qt.createComponent("../CustomTabButton.qml");

    var objTab = Tabcomponent.createObject(bar,{
                                               text: "RNAM Task #" + totalTaskRNAM,
                                               rnamObj: objRnam
                                           });
    bar.currentIndex = bar.count - 1;
}

function endTaskTNAM(obj){
    if(obj.TabBar.index == bar.currentIndex){
        bar.incrementCurrentIndex();
    }
    obj.tnamObj.destroy();
    obj.destroy();

}

function endTaskRNAM(obj){
    if(obj.TabBar.index == bar.currentIndex){
        bar.incrementCurrentIndex();
    }
    obj.rnamObj.destroy();
    obj.destroy();
}
